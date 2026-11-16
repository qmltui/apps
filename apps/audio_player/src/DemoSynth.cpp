#include "DemoSynth.h"

#include <algorithm>
#include <cmath>

namespace cyber {

namespace {
constexpr double kPi = 3.14159265358979323846;
} // namespace

DemoTrackInfo DemoSynth::trackInfo(int preset) {
    switch (std::clamp(preset, 0, kDemoTrackCount - 1)) {
    case 0:
        return {QStringLiteral("Synthwave Runner"), QStringLiteral("Neon Pulse"), 138, 165000, 320};
    case 1:
        return {QStringLiteral("Acid Overdrive"), QStringLiteral("SubGrid"), 145, 118000, 256};
    case 2:
        return {QStringLiteral("Nightcity Sub Protocol"), QStringLiteral("CyberCore"), 110, 200000,
                320};
    case 3:
        return {QStringLiteral("Phosphor Horizon"), QStringLiteral("Demoscene"), 126, 150000, 192};
    default:
        return {QStringLiteral("Bitcrushed Pulse"), QStringLiteral("Chiptune Matrix"), 160, 132000,
                320};
    }
}

void DemoSynth::reset() {
    const int savedPreset = m_preset;
    *this = DemoSynth();
    m_preset = savedPreset;
}

void DemoSynth::setTrackPreset(int preset) {
    m_preset = std::clamp(preset, 0, kDemoTrackCount - 1);
    m_tick = 0;
    m_beatIndex = -1;
    m_beatPulse = 0.f;
    m_synthTime = 0.0;
    m_wavePhase = 0.0;
}

void DemoSynth::setTickCount(int tick) {
    m_tick = std::max(0, tick);
    m_synthTime = m_tick * 0.06;
    m_wavePhase = std::fmod(m_tick * 0.18, 2.0 * kPi);
}

DemoSynth::Frame DemoSynth::step() {
    constexpr double kTwoPi = 2.0 * kPi;

    // Preset-specific timing & harmonic multipliers; preset 0 is the exact original 138 BPM track.
    const int ticksPerBeat = (m_preset == 0)   ? 13
                             : (m_preset == 1) ? 12
                             : (m_preset == 2) ? 16
                             : (m_preset == 3) ? 14
                                               : 11;
    const double timeStep = (m_preset == 0) ? 0.06 : 0.06 * (13.0 / ticksPerBeat);
    const double phaseStep = (m_preset == 0) ? 0.18 : (0.15 + m_preset * 0.04);

    ++m_tick;
    m_synthTime += timeStep;
    m_wavePhase = std::fmod(m_wavePhase + phaseStep, kTwoPi);

    const int beatStep = (m_tick / ticksPerBeat) % 4;
    if (beatStep != m_beatIndex) {
        m_beatIndex = beatStep;
        m_beatPulse = 1.f;
    } else {
        m_beatPulse = std::max(0.f, m_beatPulse - 0.12f);
    }

    const bool isKick = (m_beatIndex == 0 || m_beatIndex == 2);
    const bool isSnare = (m_beatIndex == 1 || m_beatIndex == 3);

    Frame frame;
    frame.beatPulse = m_beatPulse;

    const double bassFreq = (m_preset == 2) ? 2.4 : (m_preset == 1) ? 4.8 : 3.5;
    const double arpFreq = (m_preset == 4) ? 9.5 : (m_preset == 1) ? 7.5 : 6.0;
    const double bassBoost = (m_preset == 2) ? 1.18 : 1.0;
    const double acidBoost = (m_preset == 1) ? 1.22 : 1.0;
    const double chipBoost = (m_preset == 4) ? 1.25 : 1.0;

    for (int i = 0; i < kBandCount; ++i) {
        double base;
        if (i < 6) {
            // Sub-bass & bass: kick drum plus bass line
            const double kickEnergy = isKick ? (0.75 - i * 0.08) * m_beatPulse : 0.2;
            const double bassLine = std::abs(std::sin(m_synthTime * bassFreq + i * 0.4)) * 0.45;
            base = (kickEnergy + bassLine) * bassBoost;
        } else if (i < 14) {
            // Low-mid & mid: snare plus melodic arp
            const double snareEnergy = isSnare ? 0.65 * m_beatPulse : 0.15;
            const double arpPhase = std::sin(m_synthTime * arpFreq + i * (0.8 + m_preset * 0.12));
            const double synthArp = arpPhase > 0.3 ? arpPhase * 0.55 : 0.1;
            base = (snareEnergy + synthArp) * acidBoost;
        } else {
            // High-mid & treble: hi-hats, noise and air
            const double hiHat =
                std::abs(std::cos(m_synthTime * (12.0 + m_preset * 1.5) + i)) * 0.4;
            const double air = std::abs(std::sin(m_synthTime * 2.0 + i * 0.5)) * 0.25;
            base = (hiHat + air) * chipBoost;
        }
        frame.bandTargets[i] = float(std::clamp(base, 0.03, 1.0));
    }

    // Synthetic stereo waveform: the right channel is shifted by a quarter turn so the
    // Lissajous view draws a rotating loop instead of a diagonal line.
    const double harmonicRatio = (m_preset == 0) ? 2.2 : (1.8 + m_preset * 0.5);
    const double cycles = (m_preset == 2) ? 3.0 : (m_preset == 4) ? 6.0 : 4.0;
    for (int i = 0; i < kScopePoints; ++i) {
        const double x = (double(i) / kScopePoints) * cycles * kPi;
        double l =
            std::sin(x + m_wavePhase) + 0.4 * std::sin(x * harmonicRatio - m_wavePhase * 1.4);
        double r = std::sin(x + m_wavePhase + kPi / 2) +
                   0.4 * std::sin(x * harmonicRatio - m_wavePhase * 1.4 + kPi / 2);
        if (m_preset == 4) {
            // Quantize slightly for chiptune character on preset 4
            l = std::round(l * 4.0) / 4.0;
            r = std::round(r * 4.0) / 4.0;
        }
        frame.scopeLeft[i] = float(std::clamp(l / 1.4 * 0.85, -1.0, 1.0));
        frame.scopeRight[i] = float(std::clamp(r / 1.4 * 0.85, -1.0, 1.0));
    }

    return frame;
}

} // namespace cyber
