# Terminal Audio Player & Visualizer (`audio_player`)

A retained-mode terminal audio player and real-time spectrum/oscilloscope visualizer built on QmlTui. It uses the terminal's default palette (`"default"` + 16-color `"ansi:*"` colors) and arranges three docked decks in a responsive split layout:

1. **Main Player Deck (`MainDeck.qml`):** Play/pause/stop LED status, digital elapsed/remaining clock, clickable Mini-Visualizer (`Mini Spectrum` / `Mini Scope` / `Status`), scrolling LED track marquee, `kbps` / `kHz` / `MONO` / `STEREO` badges, interactive `VOL` and `BAL` sliders, Seek bar, Transport buttons (`Z X C V B L`), `SHUF` & `REP` toggles, and `[PL]` `[VIS]` deck switches.
2. **Playlist Editor (`PlaylistDeck.qml`):** Scrollable tracklist (`1. Artist - Title   02:45`) with active playing track indicator (`▶`), cursor selection, mouse click / `Up` / `Down` / `Enter` / `Delete` controls, total duration summary, and an interactive **Load / Add Media Modal (`AddTrackDialog.qml`)** (`L` key) to load files, folders, stream URLs, or the 5-track procedural Synthwave Demo Album.
3. **Visualizer Deck (`VisualizerDeck.qml`):** High-resolution 24-band FFT Spectrum Analyzer (`1/8`-cell resolution with peak-hold caps), Braille Stereo Oscilloscope (`WaveformTrace`), and CRT Phosphor Persistence (`PersistenceTrace`), plus Stereo VU meters and L/R phase correlation.

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
| `1` / `2` / `3` | Toggle the Main Player (`1`), Playlist (`2`), and Visualizer (`3`) decks |
| `W` | Cycle Visualizer mode: `Spectrum + Scope` → `24-Band Spectrum` → `Phosphor Scope` |
| `[` / `]` | Oscilloscope low-pass filter (`off` → `8 kHz` → `4 kHz` → `2 kHz` → `1 kHz` → `600` → `300` → `150 Hz`) |
| `A` | Toggle Braille cell color averaging on shared oscilloscope dots |
| `K` | Cycle ANSI Skins: `CLASSIC` → `AMBER AMP` → `CYBER NEON` |
| `Q` / `Esc` | Contextual Escape (closes modal → quits application) |

## Building and Testing

```bash
cmake --build build/Desktop_Debug --target qmltui_audio_player test_audio_player_dsp
ctest --test-dir build/Desktop_Debug -R 'audio_player' --output-on-failure
```

- `audio_player_dsp`: Qt Test suite covering band geometry, FFT, tone detection, RMS, trigger-aligned scope, Butterworth low-pass filter, deterministic synthwave album, stereo balance, playlist scanning/editing, and Braille canvas visualizers.
- `audio_player_demo_smoke`: Runner scenario verifying the docked decks, transport, playlist navigation, Load Media dialog, ANSI skins, and visualizer modes.
