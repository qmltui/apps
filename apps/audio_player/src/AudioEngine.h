#pragma once

#include "DemoSynth.h"
#include "SpectrumAnalyzer.h"

#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVariantList>

#include <array>
#include <vector>

#if AUDIO_PLAYER_HAVE_MULTIMEDIA
#include <QAudioFormat>
#include <QMediaPlayer>
class QAudioBuffer;
class QAudioBufferOutput;
class QAudioOutput;
#endif

// Feeds the audio player TUI from either the procedural synthwave album or media files,
// applies Stereo Balance, and owns all level smoothing so QML only renders. Every level
// property notifies through a single levelsChanged() per 33 ms tick.
class AudioEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    // Idle: nothing selected yet (start-up without media). Demo is the procedural
    // synth used by the test scenarios, the --demo flag, and the built-in demo album.
    enum class Source { Idle, Demo, File };
    Q_ENUM(Source)

    Q_PROPERTY(Source source READ source NOTIFY sourceChanged)
    Q_PROPERTY(QString sourceName READ sourceName NOTIFY sourceChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(bool stopped READ stopped NOTIFY playingChanged)
    Q_PROPERTY(bool multimediaAvailable READ multimediaAvailable CONSTANT)
    Q_PROPERTY(int bandCount READ bandCount CONSTANT)
    Q_PROPERTY(QStringList bandLabels READ bandLabels CONSTANT)

    Q_PROPERTY(QVariantList bandLevels READ bandLevels NOTIFY levelsChanged)
    Q_PROPERTY(QVariantList peakLevels READ peakLevels NOTIFY levelsChanged)
    Q_PROPERTY(qreal vuLeft READ vuLeft NOTIFY levelsChanged)
    Q_PROPERTY(qreal vuRight READ vuRight NOTIFY levelsChanged)
    Q_PROPERTY(qreal beatPulse READ beatPulse NOTIFY levelsChanged)
    // Plain double lists: QML hands them to WaveformTrace without per-element boxing.
    Q_PROPERTY(QList<double> waveformLeft READ waveformLeft NOTIFY levelsChanged)
    Q_PROPERTY(QList<double> waveformRight READ waveformRight NOTIFY levelsChanged)
    Q_PROPERTY(QList<double> waveformMono READ waveformMono NOTIFY levelsChanged)
    // Normalized L/R correlation of the scope window: +1 mono, 0 unrelated, -1 out of phase.
    Q_PROPERTY(qreal correlation READ correlation NOTIFY levelsChanged)
    // Scope display low-pass cutoff in Hz; 0 = off. Stepped through presets from QML.
    Q_PROPERTY(qreal scopeLowPassHz READ scopeLowPassHz WRITE setScopeLowPassHz NOTIFY scopeLowPassChanged)
    Q_PROPERTY(QString scopeLowPassLabel READ scopeLowPassLabel NOTIFY scopeLowPassChanged)

    // Transport & Telemetry
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 displayPosition READ displayPosition NOTIFY positionChanged)
    Q_PROPERTY(qint64 displayDuration READ displayDuration NOTIFY durationChanged)
    Q_PROPERTY(bool seekable READ seekable NOTIFY durationChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(qreal balance READ balance WRITE setBalance NOTIFY balanceChanged)
    Q_PROPERTY(bool shuffle READ shuffle WRITE setShuffle NOTIFY playbackModeChanged)
    Q_PROPERTY(bool repeat READ repeat WRITE setRepeat NOTIFY playbackModeChanged)
    Q_PROPERTY(int bitrateKbps READ bitrateKbps NOTIFY trackChanged)
    Q_PROPERTY(int sampleRateKhz READ sampleRateKhz NOTIFY trackChanged)
    Q_PROPERTY(bool stereo READ stereo NOTIFY trackChanged)

    // Track & Playlist
    Q_PROPERTY(int trackIndex READ trackIndex NOTIFY trackChanged)
    Q_PROPERTY(int trackCount READ trackCount NOTIFY trackChanged)
    Q_PROPERTY(int currentPlaylistIndex READ currentPlaylistIndex NOTIFY playlistChanged)
    Q_PROPERTY(int playlistCount READ playlistCount NOTIFY playlistChanged)
    Q_PROPERTY(int selectedTrackIndex READ selectedTrackIndex WRITE setSelectedTrackIndex NOTIFY selectedTrackChanged)
    Q_PROPERTY(QVariantList playlistModel READ playlistModel NOTIFY playlistChanged)
    Q_PROPERTY(QString totalDurationText READ totalDurationText NOTIFY playlistChanged)
    Q_PROPERTY(QString title READ title NOTIFY trackChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY trackChanged)
    Q_PROPERTY(QString marqueeString READ marqueeString NOTIFY trackChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY statusChanged)

    explicit AudioEngine(QObject* parent = nullptr);
    ~AudioEngine() override;

    Source source() const { return m_source; }
    QString sourceName() const;
    bool playing() const { return m_playing; }
    bool stopped() const { return m_stopped; }
    static bool multimediaAvailable() { return AUDIO_PLAYER_HAVE_MULTIMEDIA != 0; }
    static int bandCount() { return cyber::kBandCount; }
    static QStringList bandLabels() { return cyber::SpectrumAnalyzer::bandLabels(); }

    QVariantList bandLevels() const { return m_bandLevels; }
    QVariantList peakLevels() const { return m_peakLevels; }
    qreal vuLeft() const { return m_vuLeft; }
    qreal vuRight() const { return m_vuRight; }
    QList<double> waveformMono() const { return m_waveformMono; }
    qreal correlation() const { return m_correlation; }
    qreal beatPulse() const { return m_beatPulse; }
    QList<double> waveformLeft() const { return m_waveformLeft; }
    QList<double> waveformRight() const { return m_waveformRight; }
    qreal scopeLowPassHz() const { return m_analyzer.scopeLowPassHz(); }
    void setScopeLowPassHz(qreal cutoffHz);
    QString scopeLowPassLabel() const;

    qint64 position() const { return m_position; }
    qint64 duration() const { return m_duration; }
    qint64 displayPosition() const;
    qint64 displayDuration() const;
    bool seekable() const { return m_seekable; }
    qreal volume() const { return m_volume; }
    void setVolume(qreal volume);
    qreal balance() const { return m_balance; }
    void setBalance(qreal balance);
    bool shuffle() const { return m_shuffle; }
    void setShuffle(bool shuffle);
    bool repeat() const { return m_repeat; }
    void setRepeat(bool repeat);
    int bitrateKbps() const { return m_bitrateKbps; }
    int sampleRateKhz() const { return std::max(1, m_analyzer.sampleRate() / 1000); }
    bool stereo() const { return m_stereo; }

    int trackIndex() const { return m_trackIndex; }
    int trackCount() const { return int(m_playlist.size()); }
    int currentPlaylistIndex() const;
    int playlistCount() const { return int(m_playlistModel.size()); }
    int selectedTrackIndex() const { return m_selectedTrackIndex; }
    void setSelectedTrackIndex(int index);
    QVariantList playlistModel() const { return m_playlistModel; }
    QString totalDurationText() const { return m_totalDurationText; }
    QString title() const { return m_title; }
    QString artist() const { return m_artist; }
    QString marqueeString() const;
    QString statusText() const { return m_statusText; }
    QString errorText() const { return m_errorText; }

    // Core & Transport Invokables
    Q_INVOKABLE void loadPlaylist(const QStringList& inputs);
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void togglePlayback();
    Q_INVOKABLE void seek(qint64 deltaMs);
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void adjustVolume(qreal delta);
    Q_INVOKABLE void adjustBalance(qreal delta);
    Q_INVOKABLE void useDemo();
    Q_INVOKABLE void useFile();
    Q_INVOKABLE void stepScopeLowPass(int direction);   // -1 = lower cutoff (more filtering), +1 = higher / off
    Q_INVOKABLE static QString formatTime(qint64 ms);

    // Playlist & Track Transport Invokables
    Q_INVOKABLE void nextTrack();
    Q_INVOKABLE void previousTrack();
    Q_INVOKABLE void seekTrack(qint64 deltaMs);
    Q_INVOKABLE void seekToRatio(qreal ratio);
    Q_INVOKABLE void toggleShuffle() { setShuffle(!m_shuffle); }
    Q_INVOKABLE void toggleRepeat() { setRepeat(!m_repeat); }
    Q_INVOKABLE void playTrackAt(int index);
    Q_INVOKABLE void playSelectedTrack();
    Q_INVOKABLE void moveSelection(int delta);
    Q_INVOKABLE void removeTrackAt(int index);
    Q_INVOKABLE void removeSelectedTrack();
    Q_INVOKABLE void clearPlaylist();
    Q_INVOKABLE void appendPlaylistInputs(const QStringList& inputs);
    Q_INVOKABLE void loadDemoAlbum();

signals:
    void sourceChanged();
    void playingChanged();
    void levelsChanged();
    void scopeLowPassChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void balanceChanged();
    void playbackModeChanged();
    void trackChanged();
    void playlistChanged();
    void selectedTrackChanged();
    void statusChanged();

private:
    void tick();
    void setPlaying(bool playing);
    void setSource(Source source);
    void setStatus(const QString& status);
    void setError(const QString& error);
    void clearError();
    void resetLevels();
    void updateMonoAndCorrelation();
    void stopSources();
    void loadTrack(int index);
    void loadDemoTrack(int presetIndex);
    void rebuildPlaylistModel();
    void setPosition(qint64 position);
    void setDuration(qint64 duration, bool seekable);

    cyber::SpectrumAnalyzer m_analyzer;
    cyber::DemoSynth m_synth;
    QTimer m_timer;

    Source m_source = Source::Idle;
    bool m_playing = false;
    bool m_stopped = true;

    std::array<float, cyber::kBandCount> m_levels{};
    std::array<float, cyber::kBandCount> m_peaks{};
    std::array<float, cyber::kScopePoints> m_scopeLeft{};
    std::array<float, cyber::kScopePoints> m_scopeRight{};
    float m_vuLeft = 0.f;
    float m_vuRight = 0.f;
    float m_beatPulse = 0.f;
    QVariantList m_bandLevels;
    QVariantList m_peakLevels;
    QList<double> m_waveformLeft;
    QList<double> m_waveformRight;
    QList<double> m_waveformMono;
    float m_correlation = 1.f;

    // Playlist & Track metadata
    QList<QUrl> m_playlist;
    int m_trackIndex = -1;
    int m_demoTrackIndex = 0;
    int m_demoTrackCount = cyber::kDemoTrackCount;
    int m_selectedTrackIndex = 0;
    int m_failedInARow = 0;
    uint32_t m_shuffleSeed = 0x9E3779B9u;
    QVariantList m_playlistModel;
    QString m_totalDurationText = QStringLiteral("00:00");
    QString m_title;
    QString m_artist;
    QString m_statusText;
    QString m_errorText;
    qint64 m_position = 0;
    qint64 m_duration = 0;
    qint64 m_demoPositionMs = 0;
    qint64 m_demoDurationMs = 165000;
    bool m_seekable = false;
    qreal m_volume = 0.8;
    qreal m_balance = 0.0;
    bool m_shuffle = false;
    bool m_repeat = true;
    int m_bitrateKbps = 320;
    bool m_stereo = true;

#if AUDIO_PLAYER_HAVE_MULTIMEDIA
    void ensurePlayer();
    void ingestBuffer(const QAudioBuffer& buffer);
    void onMediaStatus(QMediaPlayer::MediaStatus status);
    void onPlayerError(QMediaPlayer::Error error, const QString& message);
    void onMetaData();

    QMediaPlayer* m_player = nullptr;
    QAudioOutput* m_audioOutput = nullptr;
    QAudioBufferOutput* m_bufferOutput = nullptr;
    std::vector<float> m_convert;
    int m_buffersForTrack = 0;
#endif

    // Decoded PCM waits here and is fed to the analyzer at real-time pace, one tick's
    // worth per tick, so bursts of large decoder buffers do not make the display flicker.
    void enqueuePcm(const float* interleavedStereo, int frames);
    std::vector<float> m_pending;          // interleaved stereo
    size_t m_pendingRead = 0;
    int m_staleTicks = 0;
    std::array<float, cyber::kBandCount> m_lastTargets{};
    float m_lastVuLeft = 0.f;
    float m_lastVuRight = 0.f;
};
