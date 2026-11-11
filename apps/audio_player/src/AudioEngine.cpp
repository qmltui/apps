#include "AudioEngine.h"

#include "Playlist.h"

#include <QVariantMap>

#include <algorithm>
#include <cmath>
#include <numbers>

#if AUDIO_PLAYER_HAVE_MULTIMEDIA
#include <QAudioBuffer>
#include <QAudioBufferOutput>
#include <QAudioOutput>
#include <QMediaMetaData>
#endif

using namespace cyber;

namespace {

constexpr int kTickMs = 33;
constexpr float kBandAttack = 0.6f;
constexpr float kBandRelease = 0.2f;
constexpr float kPeakGravity = 0.02f;
constexpr float kVuAttack = 0.65f;
constexpr float kVuRelease = 0.12f;
constexpr float kScopeFade = 0.8f;
constexpr float kScopeBlend = 0.6f;          // weight of the newest trigger-aligned scope window
constexpr float kIdleThreshold = 0.005f;
constexpr int kMaxStaleTicks = 6;            // ~200 ms without PCM before levels decay
constexpr size_t kMaxPendingSeconds = 1;     // drop older PCM beyond this backlog
// Scope low-pass presets, highest first; 0 means off.
constexpr double kLowPassPresets[] = { 0.0, 8000.0, 4000.0, 2000.0, 1000.0, 600.0, 300.0, 150.0 };
constexpr int kLowPassPresetCount = int(sizeof(kLowPassPresets) / sizeof(kLowPassPresets[0]));

template <size_t N>
QVariantList toVariantList(const std::array<float, N>& values)
{
    QVariantList list;
    list.reserve(int(N));
    for (float v : values)
        list.append(double(v));
    return list;
}

template <size_t N>
QList<double> toDoubleList(const std::array<float, N>& values)
{
    QList<double> list;
    list.reserve(int(N));
    for (float v : values)
        list.append(double(v));
    return list;
}

} // namespace

AudioEngine::AudioEngine(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(kTickMs);
    connect(&m_timer, &QTimer::timeout, this, &AudioEngine::tick);
    resetLevels();
    rebuildPlaylistModel();
    setStatus(QStringLiteral("NO SOURCE │ pass audio files or a folder on the command line, or press L to load media"));
}

AudioEngine::~AudioEngine()
{
    stopSources();
}

QString AudioEngine::sourceName() const
{
    switch (m_source) {
    case Source::File: return QStringLiteral("FILE");
    case Source::Demo: return QStringLiteral("DEMO");
    case Source::Idle: break;
    }
    return QStringLiteral("IDLE");
}

qint64 AudioEngine::displayPosition() const
{
    if (m_source == Source::Demo)
        return m_demoPositionMs;
    return m_position;
}

qint64 AudioEngine::displayDuration() const
{
    if (m_source == Source::Demo)
        return m_demoDurationMs;
    return m_duration;
}

int AudioEngine::currentPlaylistIndex() const
{
    if (m_source == Source::File && m_trackIndex >= 0)
        return m_trackIndex;
    if (m_source == Source::Demo)
        return std::clamp(m_demoTrackIndex, 0, std::max(0, m_demoTrackCount - 1));
    return -1;
}

QString AudioEngine::marqueeString() const
{
    if (m_source == Source::Idle)
        return QStringLiteral("AUDIO PLAYER // TERMINAL AUDIO PLAYER *** PRESS L TO LOAD TRACKS ***");
    const int num = currentPlaylistIndex() + 1;
    const QString artistPart = m_artist.isEmpty() ? QString() : (m_artist + QStringLiteral(" - "));
    const QString durPart = displayDuration() > 0
        ? QStringLiteral(" (%1)").arg(formatTime(displayDuration()))
        : QString();
    return QStringLiteral("%1. %2%3%4  ***  ")
        .arg(std::max(1, num))
        .arg(artistPart, m_title.isEmpty() ? QStringLiteral("Unknown Track") : m_title, durPart);
}

// ---------------------------------------------------------------------------
// Tick: smoothing + Balance shared by every source
// ---------------------------------------------------------------------------

void AudioEngine::tick()
{
    std::array<float, kBandCount> targets{};
    float vuTargetLeft = 0.f;
    float vuTargetRight = 0.f;
    bool haveWaveform = false;

    const float leftBalGain = m_balance > 0.0 ? float(1.0 - m_balance) : 1.0f;
    const float rightBalGain = m_balance < 0.0 ? float(1.0 + m_balance) : 1.0f;

    if (m_source == Source::Idle) {
        m_beatPulse = 0.f;
    } else if (m_source == Source::Demo) {
        if (m_playing) {
            const DemoSynth::Frame frame = m_synth.step();
            targets = frame.bandTargets;
            m_beatPulse = frame.beatPulse;
            m_scopeLeft = frame.scopeLeft;
            m_scopeRight = frame.scopeRight;
            haveWaveform = true;

            // Advance demo track position and auto-advance at track end
            m_demoPositionMs += kTickMs;
            if (m_demoDurationMs > 0 && m_demoPositionMs >= m_demoDurationMs) {
                if (m_repeat || m_demoTrackIndex + 1 < m_demoTrackCount) {
                    nextTrack();
                    return;
                } else {
                    stop();
                    return;
                }
            }
            emit positionChanged();
        } else {
            m_beatPulse = std::max(0.f, m_beatPulse - 0.12f);
        }
    } else {
        m_beatPulse = 0.f;
        if (m_playing) {
            // Feed one tick's worth of queued PCM to the analyzer.
            const size_t framesPerTick = size_t(m_analyzer.sampleRate()) * kTickMs / 1000;
            const size_t available = (m_pending.size() - m_pendingRead) / 2;
            const size_t take = std::min(available, framesPerTick);
            if (take > 0) {
                m_analyzer.pushInterleaved(m_pending.data() + m_pendingRead, int(take), 2);
                m_pendingRead += take * 2;
                if (m_pendingRead >= m_pending.size()) {
                    m_pending.clear();
                    m_pendingRead = 0;
                } else if (m_pendingRead > m_pending.size() / 2) {
                    m_pending.erase(m_pending.begin(), m_pending.begin() + qsizetype(m_pendingRead));
                    m_pendingRead = 0;
                }
            }

            if (m_analyzer.hasNewData()) {
                const Analysis analysis = m_analyzer.analyze();
                m_lastTargets = analysis.bands;
                // A full-scale sine has RMS 1/sqrt(2); scale so it reads 0 dB on the meter.
                m_lastVuLeft = std::min(1.f, analysis.rmsLeft * std::numbers::sqrt2_v<float>);
                m_lastVuRight = std::min(1.f, analysis.rmsRight * std::numbers::sqrt2_v<float>);
                // Windows are trigger-aligned, so blending consecutive ones averages
                // out sample noise without smearing the waveform shape.
                const float blend = m_staleTicks > kMaxStaleTicks ? 1.f : kScopeBlend;
                for (int i = 0; i < kScopePoints; ++i) {
                    m_scopeLeft[i] += (analysis.scopeLeft[i] - m_scopeLeft[i]) * blend;
                    m_scopeRight[i] += (analysis.scopeRight[i] - m_scopeRight[i]) * blend;
                }
                m_staleTicks = 0;
                haveWaveform = true;
            } else {
                ++m_staleTicks;
            }
            // Hold the last analysis briefly across gaps in delivery; decay after that.
            if (m_staleTicks <= kMaxStaleTicks) {
                targets = m_lastTargets;
                vuTargetLeft = m_lastVuLeft;
                vuTargetRight = m_lastVuRight;
                haveWaveform = haveWaveform || m_staleTicks > 0;
            }
        }
    }

    bool active = false;
    for (int i = 0; i < kBandCount; ++i) {
        const float target = targets[i];
        float& level = m_levels[i];
        const float rate = target > level ? kBandAttack : kBandRelease;
        level += (target - level) * rate;
        if (level < kIdleThreshold)
            level = 0.f;
        float& peak = m_peaks[i];
        peak = level >= peak ? level : std::max(level, peak - kPeakGravity);
        if (peak < kIdleThreshold)
            peak = 0.f;
        active = active || level > 0.f || peak > 0.f;
    }

    if (m_source == Source::Demo && m_playing) {
        vuTargetLeft = std::min(1.f, m_levels[1] * 0.55f + m_levels[8] * 0.35f + m_levels[16] * 0.10f);
        vuTargetRight = std::min(1.f, m_levels[2] * 0.50f + m_levels[10] * 0.35f + m_levels[18] * 0.15f);
    }
    vuTargetLeft *= leftBalGain;
    vuTargetRight *= rightBalGain;

    auto smoothVu = [](float& value, float target) {
        value = target > value ? value * (1.f - kVuAttack) + target * kVuAttack
                               : value * (1.f - kVuRelease) + target * kVuRelease;
        if (value < kIdleThreshold)
            value = 0.f;
    };
    smoothVu(m_vuLeft, vuTargetLeft);
    smoothVu(m_vuRight, vuTargetRight);
    m_bassEnergy = std::clamp(
        m_levels[0] * 0.35f + m_levels[1] * 0.30f + m_levels[2] * 0.20f + m_levels[3] * 0.15f,
        0.f, 1.f);
    active = active || m_vuLeft > 0.f || m_vuRight > 0.f || m_beatPulse > 0.f;

    if (haveWaveform) {
        for (int i = 0; i < kScopePoints; ++i) {
            m_scopeLeft[i] = std::clamp(m_scopeLeft[i] * leftBalGain, -1.f, 1.f);
            m_scopeRight[i] = std::clamp(m_scopeRight[i] * rightBalGain, -1.f, 1.f);
        }
    } else {
        bool scopeActive = false;
        for (int i = 0; i < kScopePoints; ++i) {
            m_scopeLeft[i] *= kScopeFade;
            m_scopeRight[i] *= kScopeFade;
            if (std::abs(m_scopeLeft[i]) < kIdleThreshold)
                m_scopeLeft[i] = 0.f;
            if (std::abs(m_scopeRight[i]) < kIdleThreshold)
                m_scopeRight[i] = 0.f;
            scopeActive = scopeActive || m_scopeLeft[i] != 0.f || m_scopeRight[i] != 0.f;
        }
        active = active || scopeActive;
    }

    m_bandLevels = toVariantList(m_levels);
    m_peakLevels = toVariantList(m_peaks);
    m_waveformLeft = toDoubleList(m_scopeLeft);
    m_waveformRight = toDoubleList(m_scopeRight);
    updateMonoAndCorrelation();
    emit levelsChanged();

    // Once paused/stopped output has settled, stop ticking so the frame stays quiet.
    if (!m_playing && !active)
        m_timer.stop();
}

void AudioEngine::resetLevels()
{
    m_levels.fill(0.f);
    m_peaks.fill(0.f);
    m_scopeLeft.fill(0.f);
    m_scopeRight.fill(0.f);
    m_vuLeft = m_vuRight = m_bassEnergy = m_beatPulse = 0.f;
    m_bandLevels = toVariantList(m_levels);
    m_peakLevels = toVariantList(m_peaks);
    m_waveformLeft = toDoubleList(m_scopeLeft);
    m_waveformRight = toDoubleList(m_scopeRight);
    updateMonoAndCorrelation();
    emit levelsChanged();
}

void AudioEngine::updateMonoAndCorrelation()
{
    m_waveformMono.resize(kScopePoints);
    double sumLR = 0.0, sumLL = 0.0, sumRR = 0.0;
    for (int i = 0; i < kScopePoints; ++i) {
        const double l = m_scopeLeft[i];
        const double r = m_scopeRight[i];
        m_waveformMono[i] = std::clamp((l + r) * 0.70710678, -1.0, 1.0);
        sumLR += l * r;
        sumLL += l * l;
        sumRR += r * r;
    }
    const double denom = std::sqrt(sumLL * sumRR);
    const float target = denom > 1e-9 ? float(std::clamp(sumLR / denom, -1.0, 1.0)) : m_correlation;
    m_correlation += (target - m_correlation) * 0.25f;   // light smoothing for a readable number
}

// ---------------------------------------------------------------------------
// State helpers
// ---------------------------------------------------------------------------

void AudioEngine::setPlaying(bool playing)
{
    const bool wasStopped = m_stopped;
    if (playing)
        m_stopped = false;
    const bool changed = (m_playing != playing) || (wasStopped != m_stopped);
    m_playing = playing;
    if (changed)
        emit playingChanged();
    if (playing) {
        // Produce data synchronously so the very first frame after a source change
        // already shows levels instead of waiting for the first timer tick.
        tick();
        m_timer.start();
    }
}

void AudioEngine::setSource(Source source)
{
    if (m_source == source)
        return;
    m_source = source;
    emit sourceChanged();
}

void AudioEngine::setStatus(const QString& status)
{
    if (m_statusText == status)
        return;
    m_statusText = status;
    emit statusChanged();
}

void AudioEngine::setError(const QString& error)
{
    if (m_errorText == error)
        return;
    m_errorText = error;
    emit statusChanged();
}

void AudioEngine::clearError()
{
    setError(QString());
}

void AudioEngine::setPosition(qint64 position)
{
    if (m_position == position)
        return;
    m_position = position;
    emit positionChanged();
}

void AudioEngine::setDuration(qint64 duration, bool seekable)
{
    if (m_duration == duration && m_seekable == seekable)
        return;
    m_duration = duration;
    m_seekable = seekable;
    emit durationChanged();
}

void AudioEngine::setVolume(qreal volume)
{
    const qreal clamped = std::clamp(volume, 0.0, 1.0);
    if (qFuzzyCompare(m_volume, clamped))
        return;
    m_volume = clamped;
#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    if (m_audioOutput)
        m_audioOutput->setVolume(float(m_volume));
#endif
    emit volumeChanged();
}

void AudioEngine::adjustVolume(qreal delta)
{
    setVolume(m_volume + delta);
}

void AudioEngine::setBalance(qreal balance)
{
    const qreal clamped = std::clamp(balance, -1.0, 1.0);
    if (qFuzzyCompare(m_balance, clamped))
        return;
    m_balance = clamped;
    emit balanceChanged();
}

void AudioEngine::adjustBalance(qreal delta)
{
    setBalance(m_balance + delta);
}

void AudioEngine::setShuffle(bool shuffle)
{
    if (m_shuffle == shuffle)
        return;
    m_shuffle = shuffle;
    emit playbackModeChanged();
}

void AudioEngine::setRepeat(bool repeat)
{
    if (m_repeat == repeat)
        return;
    m_repeat = repeat;
    emit playbackModeChanged();
}

void AudioEngine::setSelectedTrackIndex(int index)
{
    const int count = playlistCount();
    const int clamped = count > 0 ? std::clamp(index, 0, count - 1) : 0;
    if (m_selectedTrackIndex == clamped)
        return;
    m_selectedTrackIndex = clamped;
    emit selectedTrackChanged();
}

void AudioEngine::setScopeLowPassHz(qreal cutoffHz)
{
    const double before = m_analyzer.scopeLowPassHz();
    m_analyzer.setScopeLowPassHz(cutoffHz);
    if (m_analyzer.scopeLowPassHz() != before)
        emit scopeLowPassChanged();
}

void AudioEngine::stepScopeLowPass(int direction)
{
    // Find the current preset (nearest), then move one step. Index 0 is "off".
    const double current = m_analyzer.scopeLowPassHz();
    int index = 0;
    for (int i = 0; i < kLowPassPresetCount; ++i)
        if (std::abs(kLowPassPresets[i] - current) < std::abs(kLowPassPresets[index] - current))
            index = i;
    // direction -1 lowers the cutoff (higher preset index); +1 raises it toward off.
    index = std::clamp(index - direction, 0, kLowPassPresetCount - 1);
    setScopeLowPassHz(kLowPassPresets[index]);
}

QString AudioEngine::scopeLowPassLabel() const
{
    const double hz = m_analyzer.scopeLowPassHz();
    if (hz <= 0.0)
        return QStringLiteral("off");
    if (hz >= 1000.0)
        return QStringLiteral("%1 kHz").arg(hz / 1000.0, 0, 'g', 3);
    return QStringLiteral("%1 Hz").arg(int(std::lround(hz)));
}

QString AudioEngine::formatTime(qint64 ms)
{
    const qint64 totalSeconds = std::max<qint64>(0, ms) / 1000;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds / 60) % 60;
    const qint64 seconds = totalSeconds % 60;
    const QString mmss = QStringLiteral("%1:%2")
                             .arg(minutes, 2, 10, QLatin1Char('0'))
                             .arg(seconds, 2, 10, QLatin1Char('0'));
    return hours > 0 ? QStringLiteral("%1:%2").arg(hours).arg(mmss) : mmss;
}

// ---------------------------------------------------------------------------
// Transport
// ---------------------------------------------------------------------------

void AudioEngine::play()
{
    if (m_source == Source::Idle)
        return;
#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    if (m_source == Source::File && m_player) {
        m_player->play();
        setPlaying(true);
        return;
    }
#endif
    setPlaying(true);
}

void AudioEngine::pause()
{
    const bool stoppedChanged = m_stopped;
    m_stopped = false;
#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    if (m_source == Source::File && m_player)
        m_player->pause();
#endif
    setPlaying(false);
    if (stoppedChanged)
        emit playingChanged();
}

void AudioEngine::stop()
{
    const bool stoppedChanged = !m_stopped;
    m_stopped = true;
#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    if (m_source == Source::File && m_player) {
        m_player->stop();
        setPosition(0);
    }
#endif
    if (m_source == Source::Demo) {
        m_demoPositionMs = 0;
        m_synth.setTrackPreset(m_demoTrackIndex);
        emit positionChanged();
    }
    setPlaying(false);
    if (stoppedChanged)
        emit playingChanged();
}

void AudioEngine::togglePlayback()
{
    if (m_playing)
        pause();
    else
        play();
}

void AudioEngine::seek(qint64 deltaMs)
{
#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    if (m_source != Source::File || !m_player || !m_seekable)
        return;
    const qint64 target = std::clamp<qint64>(m_player->position() + deltaMs, 0, std::max<qint64>(0, m_duration));
    m_player->setPosition(target);
    setPosition(target);
#else
    Q_UNUSED(deltaMs);
#endif
}

void AudioEngine::seekTrack(qint64 deltaMs)
{
    if (m_source == Source::Demo) {
        m_demoPositionMs = std::clamp<qint64>(m_demoPositionMs + deltaMs, 0, std::max<qint64>(0, m_demoDurationMs));
        m_synth.setTickCount(int(m_demoPositionMs / kTickMs));
        emit positionChanged();
        return;
    }
    seek(deltaMs);
}

void AudioEngine::seekToRatio(qreal ratio)
{
    const qreal clamped = std::clamp(ratio, 0.0, 1.0);
    if (m_source == Source::Demo && m_demoDurationMs > 0) {
        m_demoPositionMs = qint64(clamped * m_demoDurationMs);
        m_synth.setTickCount(int(m_demoPositionMs / kTickMs));
        emit positionChanged();
        return;
    }
#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    if (m_source == Source::File && m_player && m_seekable && m_duration > 0) {
        const qint64 target = qint64(clamped * m_duration);
        m_player->setPosition(target);
        setPosition(target);
    }
#endif
}

void AudioEngine::next()
{
    if (m_playlist.isEmpty())
        return;
    if (m_source != Source::File) {
        useFile();
        return;
    }
    loadTrack((m_trackIndex + 1) % int(m_playlist.size()));
}

void AudioEngine::previous()
{
    if (m_playlist.isEmpty())
        return;
    if (m_source != Source::File) {
        useFile();
        return;
    }
    loadTrack((m_trackIndex - 1 + int(m_playlist.size())) % int(m_playlist.size()));
}

void AudioEngine::nextTrack()
{
    if (!m_playlist.isEmpty()) {
        if (m_source != Source::File) {
            useFile();
            return;
        }
        const int count = int(m_playlist.size());
        if (m_shuffle && count > 1) {
            m_shuffleSeed = m_shuffleSeed * 1664525u + 1013904223u;
            int pick = int(m_shuffleSeed % uint32_t(count - 1));
            if (pick >= m_trackIndex)
                ++pick;
            loadTrack(pick);
        } else {
            loadTrack((m_trackIndex + 1) % count);
        }
        return;
    }
    if (m_source == Source::Demo && m_demoTrackCount > 0) {
        if (m_shuffle && m_demoTrackCount > 1) {
            m_shuffleSeed = m_shuffleSeed * 1664525u + 1013904223u;
            int pick = int(m_shuffleSeed % uint32_t(m_demoTrackCount - 1));
            if (pick >= m_demoTrackIndex)
                ++pick;
            loadDemoTrack(pick);
        } else {
            loadDemoTrack((m_demoTrackIndex + 1) % m_demoTrackCount);
        }
    }
}

void AudioEngine::previousTrack()
{
    if (!m_playlist.isEmpty()) {
        previous();
        return;
    }
    if (m_source == Source::Demo && m_demoTrackCount > 0)
        loadDemoTrack((m_demoTrackIndex - 1 + m_demoTrackCount) % m_demoTrackCount);
}

// ---------------------------------------------------------------------------
// Playlist Editor Management
// ---------------------------------------------------------------------------

void AudioEngine::rebuildPlaylistModel()
{
    m_playlistModel.clear();
    qint64 totalMs = 0;

    if (!m_playlist.isEmpty()) {
        for (int i = 0; i < int(m_playlist.size()); ++i) {
            const QUrl& url = m_playlist.at(i);
            const bool isCurrent = (m_source == Source::File && i == m_trackIndex);
            const QString name = isCurrent && !m_title.isEmpty() ? m_title : displayNameFor(url);
            const QString art = isCurrent ? m_artist : QString();
            const qint64 dur = isCurrent ? m_duration : 0;
            totalMs += dur;

            QVariantMap item;
            item.insert(QStringLiteral("index"), i);
            item.insert(QStringLiteral("number"), i + 1);
            item.insert(QStringLiteral("title"), name);
            item.insert(QStringLiteral("artist"), art);
            item.insert(QStringLiteral("displayTitle"),
                        art.isEmpty() ? name : (art + QStringLiteral(" - ") + name));
            item.insert(QStringLiteral("durationMs"), dur);
            item.insert(QStringLiteral("durationText"), dur > 0 ? formatTime(dur) : QStringLiteral("--:--"));
            item.insert(QStringLiteral("current"), isCurrent);
            item.insert(QStringLiteral("sourceType"), QStringLiteral("FILE"));
            m_playlistModel.append(item);
        }
    } else {
        for (int i = 0; i < m_demoTrackCount; ++i) {
            const DemoTrackInfo info = DemoSynth::trackInfo(i);
            const bool isCurrent = (m_source == Source::Demo && i == m_demoTrackIndex);
            totalMs += info.durationMs;

            QVariantMap item;
            item.insert(QStringLiteral("index"), i);
            item.insert(QStringLiteral("number"), i + 1);
            item.insert(QStringLiteral("title"), info.title);
            item.insert(QStringLiteral("artist"), info.artist);
            item.insert(QStringLiteral("displayTitle"),
                        info.artist + QStringLiteral(" - ") + info.title);
            item.insert(QStringLiteral("durationMs"), info.durationMs);
            item.insert(QStringLiteral("durationText"), formatTime(info.durationMs));
            item.insert(QStringLiteral("current"), isCurrent);
            item.insert(QStringLiteral("sourceType"), QStringLiteral("SYNTH"));
            m_playlistModel.append(item);
        }
    }

    m_totalDurationText = formatTime(totalMs);
    if (m_selectedTrackIndex >= int(m_playlistModel.size()))
        m_selectedTrackIndex = std::max(0, int(m_playlistModel.size()) - 1);
    emit playlistChanged();
}

void AudioEngine::loadDemoTrack(int presetIndex)
{
    if (m_demoTrackCount <= 0)
        m_demoTrackCount = kDemoTrackCount;
    m_demoTrackIndex = std::clamp(presetIndex, 0, m_demoTrackCount - 1);
    m_selectedTrackIndex = m_demoTrackIndex;
    const DemoTrackInfo info = DemoSynth::trackInfo(m_demoTrackIndex);
    m_title = info.title;
    m_artist = info.artist;
    m_bitrateKbps = info.kbps;
    m_stereo = true;
    m_demoPositionMs = 0;
    m_demoDurationMs = info.durationMs;
    m_synth.setTrackPreset(m_demoTrackIndex);
    m_analyzer.clear();
    setSource(Source::Demo);
    clearError();
    setStatus(QStringLiteral("DEMO │ %1 │ %2 BPM")
                  .arg(info.title.toUpper())
                  .arg(info.bpm));
    rebuildPlaylistModel();
    emit trackChanged();
    emit durationChanged();
    emit positionChanged();
    emit selectedTrackChanged();
    setPlaying(true);
}

void AudioEngine::playTrackAt(int index)
{
    if (!m_playlist.isEmpty()) {
        if (index < 0 || index >= int(m_playlist.size()))
            return;
        m_selectedTrackIndex = index;
        emit selectedTrackChanged();
        if (m_source != Source::File) {
            m_trackIndex = index;
            useFile();
        } else {
            loadTrack(index);
        }
        return;
    }
    if (index >= 0 && index < m_demoTrackCount) {
        stopSources();
        loadDemoTrack(index);
    }
}

void AudioEngine::playSelectedTrack()
{
    playTrackAt(m_selectedTrackIndex);
}

void AudioEngine::moveSelection(int delta)
{
    const int count = playlistCount();
    if (count <= 0)
        return;
    setSelectedTrackIndex(std::clamp(m_selectedTrackIndex + delta, 0, count - 1));
}

void AudioEngine::removeTrackAt(int index)
{
    if (!m_playlist.isEmpty()) {
        if (index < 0 || index >= int(m_playlist.size()))
            return;
        m_playlist.removeAt(index);
        if (m_playlist.isEmpty()) {
            stopSources();
            setSource(Source::Idle);
            m_trackIndex = -1;
            rebuildPlaylistModel();
            emit trackChanged();
            return;
        }
        if (index < m_trackIndex) {
            --m_trackIndex;
        } else if (index == m_trackIndex) {
            loadTrack(std::min(m_trackIndex, int(m_playlist.size()) - 1));
            return;
        }
        rebuildPlaylistModel();
        emit trackChanged();
        return;
    }
    if (m_demoTrackCount > 1 && index >= 0 && index < m_demoTrackCount) {
        --m_demoTrackCount;
        if (m_demoTrackIndex >= m_demoTrackCount)
            m_demoTrackIndex = m_demoTrackCount - 1;
        rebuildPlaylistModel();
        emit trackChanged();
    }
}

void AudioEngine::removeSelectedTrack()
{
    removeTrackAt(m_selectedTrackIndex);
}

void AudioEngine::clearPlaylist()
{
    m_playlist.clear();
    m_trackIndex = -1;
    m_demoTrackCount = 0;
    stopSources();
    setSource(Source::Idle);
    m_title.clear();
    m_artist.clear();
    rebuildPlaylistModel();
    setStatus(QStringLiteral("PLAYLIST CLEARED │ press L to add tracks or load the Synthwave Demo Album"));
    emit trackChanged();
}

void AudioEngine::appendPlaylistInputs(const QStringList& inputs)
{
    const PlaylistScan scan = collectPlaylist(inputs);
    if (!multimediaAvailable()) {
        setError(QStringLiteral("Qt Multimedia not available in this build; using procedural Demo Album"));
        return;
    }
    if (scan.entries.isEmpty()) {
        setError(scan.warnings.isEmpty() ? QStringLiteral("No playable files found in path")
                                         : scan.warnings.join(QStringLiteral(" · ")));
        return;
    }
    const bool wasEmpty = m_playlist.isEmpty();
    m_playlist.append(scan.entries);
    if (!scan.warnings.isEmpty())
        setError(scan.warnings.join(QStringLiteral(" · ")));
    else
        clearError();
    rebuildPlaylistModel();
    emit trackChanged();
    if (wasEmpty || m_source != Source::File)
        useFile();
}

void AudioEngine::loadDemoAlbum()
{
    m_playlist.clear();
    m_trackIndex = -1;
    m_demoTrackCount = kDemoTrackCount;
    useDemo();
}

// ---------------------------------------------------------------------------
// Sources
// ---------------------------------------------------------------------------

void AudioEngine::useDemo()
{
    stopSources();
    if (m_demoTrackCount <= 0)
        m_demoTrackCount = kDemoTrackCount;
    m_demoTrackIndex = 0;
    m_selectedTrackIndex = 0;
    const DemoTrackInfo info = DemoSynth::trackInfo(0);
    m_title = info.title;
    m_artist = info.artist;
    m_bitrateKbps = info.kbps;
    m_stereo = true;
    m_demoPositionMs = 0;
    m_demoDurationMs = info.durationMs;
    m_synth.setTrackPreset(0);
    m_synth.reset();
    m_analyzer.clear();
    setSource(Source::Demo);
    clearError();
    setStatus(QStringLiteral("DEMO │ SYNTHWAVE RUNNER │ 138 BPM"));
    rebuildPlaylistModel();
    emit trackChanged();
    emit durationChanged();
    emit positionChanged();
    setPlaying(true);
}

void AudioEngine::loadPlaylist(const QStringList& inputs)
{
    const PlaylistScan scan = collectPlaylist(inputs);
    if (!multimediaAvailable()) {
        setError(QStringLiteral("Qt Multimedia not available in this build; use --demo for the showcase"));
        return;
    }
    if (scan.entries.isEmpty()) {
        setError(scan.warnings.isEmpty() ? QStringLiteral("No playable files given")
                                         : scan.warnings.join(QStringLiteral(" · ")));
        return;
    }
    m_playlist = scan.entries;
    m_failedInARow = 0;
    if (!scan.warnings.isEmpty())
        setError(scan.warnings.join(QStringLiteral(" · ")));
    else
        clearError();
    rebuildPlaylistModel();
    useFile();
}

void AudioEngine::useFile()
{
#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    if (m_playlist.isEmpty()) {
        setError(QStringLiteral("No playlist loaded"));
        return;
    }
    stopSources();
    ensurePlayer();
    m_analyzer.setSampleRate(48000);
    m_analyzer.clear();
    setSource(Source::File);
    loadTrack(m_trackIndex < 0 ? 0 : m_trackIndex);
#else
    setError(QStringLiteral("Qt Multimedia not available in this build"));
#endif
}

void AudioEngine::stopSources()
{
    m_pending.clear();
    m_pendingRead = 0;
    m_staleTicks = kMaxStaleTicks + 1;
    m_lastTargets.fill(0.f);
    m_lastVuLeft = m_lastVuRight = 0.f;
    resetLevels();
#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    if (m_player)
        m_player->stop();
#endif
    setPosition(0);
    setDuration(0, false);
}

void AudioEngine::loadTrack(int index)
{
#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    if (!m_player || m_playlist.isEmpty())
        return;
    m_trackIndex = std::clamp(index, 0, int(m_playlist.size()) - 1);
    m_selectedTrackIndex = m_trackIndex;
    const QUrl url = m_playlist.at(m_trackIndex);
    m_title = displayNameFor(url);
    m_artist.clear();
    m_buffersForTrack = 0;
    m_analyzer.clear();
    rebuildPlaylistModel();
    emit trackChanged();
    emit selectedTrackChanged();
    setStatus(QStringLiteral("FILE │ TRACK %1/%2").arg(m_trackIndex + 1).arg(m_playlist.size()));

    m_player->setSource(url);
    m_player->play();
    setPlaying(true);

    // QAudioBufferOutput is only served by the FFmpeg backend. Tell the user when
    // playback runs but no PCM ever arrives.
    QTimer::singleShot(1500, this, [this, index = m_trackIndex] {
        if (m_source == Source::File && m_trackIndex == index && m_playing
            && m_buffersForTrack == 0 && m_errorText.isEmpty())
            setError(QStringLiteral("No audio data received: audio device busy (another player running?) "
                                    "or Qt Multimedia FFmpeg backend missing"));
    });
#else
    Q_UNUSED(index);
#endif
}

void AudioEngine::enqueuePcm(const float* interleavedStereo, int frames)
{
    if (!interleavedStereo || frames <= 0)
        return;
    m_pending.insert(m_pending.end(), interleavedStereo, interleavedStereo + size_t(frames) * 2);
    // Keep the backlog bounded so the display never lags far behind the sound.
    const size_t maxPending = size_t(m_analyzer.sampleRate()) * kMaxPendingSeconds * 2;
    if (m_pending.size() - m_pendingRead > maxPending) {
        m_pending.erase(m_pending.begin(), m_pending.end() - qsizetype(maxPending));
        m_pendingRead = 0;
    }
}

// ---------------------------------------------------------------------------
// Qt Multimedia integration
// ---------------------------------------------------------------------------

#if AUDIO_PLAYER_HAVE_MULTIMEDIA

void AudioEngine::ensurePlayer()
{
    if (m_player)
        return;
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_audioOutput->setVolume(float(m_volume));
    m_player->setAudioOutput(m_audioOutput);

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Float);
    m_bufferOutput = new QAudioBufferOutput(format, this);
    m_player->setAudioBufferOutput(m_bufferOutput);

    // Buffers are emitted from a decoder thread; this object lives on the main thread,
    // so the connection is queued and the (implicitly shared) buffer crosses safely.
    connect(m_bufferOutput, &QAudioBufferOutput::audioBufferReceived, this, &AudioEngine::ingestBuffer);
    connect(m_player, &QMediaPlayer::positionChanged, this, &AudioEngine::setPosition);
    connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 d) {
        setDuration(d, m_player->isSeekable());
        rebuildPlaylistModel();
    });
    connect(m_player, &QMediaPlayer::seekableChanged, this, [this](bool s) { setDuration(m_player->duration(), s); });
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &AudioEngine::onMediaStatus);
    connect(m_player, &QMediaPlayer::errorOccurred, this, &AudioEngine::onPlayerError);
    connect(m_player, &QMediaPlayer::metaDataChanged, this, &AudioEngine::onMetaData);
}

void AudioEngine::ingestBuffer(const QAudioBuffer& buffer)
{
    if (!buffer.isValid())
        return;
    const QAudioFormat format = buffer.format();
    const int channels = format.channelCount();
    const int frames = int(buffer.frameCount());
    if (channels <= 0 || frames <= 0)
        return;
    if (m_source == Source::File)
        ++m_buffersForTrack;
    m_failedInARow = 0;
    m_analyzer.setSampleRate(format.sampleRate());
    m_stereo = channels > 1;

    // Normalise every format to interleaved stereo float.
    m_convert.resize(size_t(frames) * 2);
    if (format.sampleFormat() == QAudioFormat::Float) {
        const float* src = buffer.constData<float>();
        for (int f = 0; f < frames; ++f) {
            const float* frame = src + qsizetype(f) * channels;
            m_convert[size_t(f) * 2] = frame[0];
            m_convert[size_t(f) * 2 + 1] = channels > 1 ? frame[1] : frame[0];
        }
    } else {
        // Generic path for Int16 / Int32 / UInt8 buffers.
        const auto* bytes = buffer.constData<char>();
        const int bytesPerSample = format.bytesPerSample();
        for (int f = 0; f < frames; ++f) {
            const char* frame = bytes + qsizetype(f) * channels * bytesPerSample;
            const float left = format.normalizedSampleValue(frame);
            const float right = channels > 1 ? format.normalizedSampleValue(frame + bytesPerSample) : left;
            m_convert[size_t(f) * 2] = left;
            m_convert[size_t(f) * 2 + 1] = right;
        }
    }
    enqueuePcm(m_convert.data(), frames);
}

void AudioEngine::onMediaStatus(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia && m_source == Source::File) {
        if (!m_repeat && !m_shuffle && m_trackIndex + 1 >= int(m_playlist.size())) {
            stop();
            return;
        }
        nextTrack();
    }
}

void AudioEngine::onPlayerError(QMediaPlayer::Error error, const QString& message)
{
    if (error == QMediaPlayer::NoError || m_source != Source::File)
        return;
    setError(QStringLiteral("ERROR │ %1 │ %2").arg(m_title, message.trimmed()));
    ++m_failedInARow;
    if (m_failedInARow >= int(m_playlist.size())) {
        pause();
        return;
    }
    QTimer::singleShot(1500, this, [this] {
        if (m_source == Source::File && !m_errorText.isEmpty())
            nextTrack();
    });
}

void AudioEngine::onMetaData()
{
    if (!m_player || m_trackIndex < 0)
        return;
    const QMediaMetaData meta = m_player->metaData();
    const QString title = meta.stringValue(QMediaMetaData::Title).trimmed();
    QString artist = meta.stringValue(QMediaMetaData::ContributingArtist).trimmed();
    if (artist.isEmpty())
        artist = meta.stringValue(QMediaMetaData::AlbumArtist).trimmed();
    if (artist.isEmpty())
        artist = meta.stringValue(QMediaMetaData::Author).trimmed();
    const int br = meta.value(QMediaMetaData::AudioBitRate).toInt();
    if (br > 0)
        m_bitrateKbps = std::max(1, br / 1000);
    const QString newTitle = title.isEmpty() ? displayNameFor(m_playlist.at(m_trackIndex)) : title;
    if (newTitle == m_title && artist == m_artist)
        return;
    m_title = newTitle;
    m_artist = artist;
    rebuildPlaylistModel();
    emit trackChanged();
}

#endif // AUDIO_PLAYER_HAVE_MULTIMEDIA
