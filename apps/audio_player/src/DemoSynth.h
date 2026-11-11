#pragma once

#include "SpectrumAnalyzer.h"

#include <QString>

#include <array>

namespace cyber {

constexpr int kDemoTrackCount = 5;

struct DemoTrackInfo {
    QString title;
    QString artist;
    int bpm = 138;
    qint64 durationMs = 165000;
    int kbps = 320;
};

// Deterministic procedural synthwave source: 5 built-in tracks with distinct BPMs,
// basslines, arp harmonics, and stereo waveforms. Track 0 is the 138 BPM reference
// synthwave track used by the deterministic unit tests.
// Produces per-band targets (already in the 0..1 level domain) and a synthetic
// stereo waveform for the oscilloscope. One step() equals one 33 ms engine tick.
class DemoSynth {
public:
    struct Frame {
        std::array<float, kBandCount> bandTargets{};
        float beatPulse = 0.f;
        std::array<float, kScopePoints> scopeLeft{};
        std::array<float, kScopePoints> scopeRight{};
    };

    static int trackCount() { return kDemoTrackCount; }
    static DemoTrackInfo trackInfo(int preset);

    void reset();
    void setTrackPreset(int preset);
    int trackPreset() const { return m_preset; }
    int tickCount() const { return m_tick; }
    void setTickCount(int tick);

    Frame step();

private:
    int m_preset = 0;
    int m_tick = 0;
    int m_beatIndex = -1;          // -1 so the very first tick lands on a beat
    float m_beatPulse = 0.f;
    double m_synthTime = 0.0;
    double m_wavePhase = 0.0;
};

} // namespace cyber
