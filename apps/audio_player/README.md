# Terminal Audio Player & Visualizer (`audio_player`)

A retained-mode terminal audio player and real-time spectrum/oscilloscope visualizer built on QmlTui. It uses strictly the terminal's default palette (`"default"` + 16-color `"ansi:*"` colors) and arranges its views in a responsive single-hairline divided grid (`Playlist │ Visualizer` above a full-width player bar):

1. **Playlist Pane (`PlaylistDeck.qml`):** Left-column scrollable tracklist (`1. Artist - Title   02:45`) with inline `+add`/`-rem`/`demo`/`clr` actions, active playing track highlight (`▶`), cursor selection, total duration summary, and a **Load / Add Media Modal (`AddTrackDialog.qml`)** (`L` key).
2. **Spectrum Pane (`SpectrumDeck.qml`):** Dedicated top-right 24-band FFT Spectrum Analyzer (`1/8`-cell resolution with peak-hold caps) and compact 1-row Stereo VU bar.
3. **Visualizer / Oscilloscope Pane (`VisualizerDeck.qml`):** Dedicated bottom-right Braille Oscilloscope panel where `W` cycles between **Stereo Scope** (`WaveformTrace`), **Phosphor Scope** (`PersistenceTrace`), and **Radial Scope** (`RadialScopeTrace` wrapping 512 samples around a bass-reactive circle with 16-sample seam blending and glowing stroke).
4. **Bottom Player Bar (`MainDeck.qml`):** Full-width 2-row deck beneath a dimmed `───┴───` hairline with play/pause/stop state, track & status readout, bitrate/sample-rate info, `SHUF`/`REP` toggles, volume/balance readouts, transport controls, and a full-width dotted seek bar.

Two audio sources feed the same engine:

| Source | Badge | How | Requires |
| :--- | :--- | :--- | :--- |
| Media files / URLs | `FILE` | `QMediaPlayer` + `QAudioOutput` for playback, `QAudioBufferOutput` for decoded PCM | Qt Multimedia with FFmpeg backend |
| Procedural Synth Album | `DEMO` | Deterministic 5-track synthwave album in C++ (`--demo` or `[DEMO]` button) | nothing |

## Usage

```bash
qmltui_audio_player [--demo] [--volume <0-100>] [media...]

qmltui_audio_player ~/Music/album/            # play a directory (recursive, sorted)
qmltui_audio_player track.flac other.mp3      # explicit playlist
qmltui_audio_player https://host/stream.mp3   # URLs are passed to QMediaPlayer
qmltui_audio_player --demo                    # 5-track procedural synthwave album
qmltui_audio_player                           # idle: press L to load media
```

## Keyboard Controls

| Key | Action |
| :--- | :--- |
| `Space` / `C` | Pause / resume playback |
| `Z` / `X` / `C` / `V` / `B` | Transport: Previous (`Z`), Play (`X`), Pause (`C`), Stop (`V`), Next (`B`) |
| `N` / `P` | Next / previous track |
| `S` / `R` | Toggle Shuffle (`SHUF`) / Repeat (`REP`) |
| `L` | Open the **Load / Add Media** modal dialog (add file, directory, URL, or Demo Album) |
| `↑` / `↓` / `Enter` / `Del` | Navigate Playlist Editor cursor, play selected track (`Enter`), or remove track (`Del`) |
| `←` / `→` | Seek 5 s backward / forward |
| `+` / `-` | Volume in 5 % steps |
| `T` | Toggle digital clock between elapsed and remaining track time |
| `1` / `2` / `3` / `4` | Toggle the Main Player (`1`), Playlist (`2`), Spectrum (`3`), and Oscilloscope (`4`) decks |
| `W` | Cycle Oscilloscope visualization mode: `Stereo Scope` → `Phosphor Scope` → `Radial Scope` |
| `A` | Toggle Braille cell color averaging on shared oscilloscope dots |
| `Q` / `Esc` | Contextual Escape (closes modal → quits application) |

## Building and Testing

```bash
cmake --build build/Desktop_Debug --target qmltui_audio_player test_audio_player_dsp
ctest --test-dir build/Desktop_Debug -R 'audio_player' --output-on-failure
```

- `audio_player_dsp`: Qt Test suite covering band geometry, FFT, tone detection, RMS, trigger-aligned scope, Butterworth low-pass filter, deterministic synthwave album, stereo balance, playlist scanning/editing, and Braille canvas visualizers.
- `audio_player_demo_smoke`: Runner scenario verifying the docked decks, transport, playlist navigation, Load Media dialog, ANSI skins, and visualizer modes.
