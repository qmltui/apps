# Terminal Audio Player & Visualizer (`audio_player`)

Winamp-inspired terminal audio player and real-time visualizer using only the terminal's default palette (`"default"` + 16-color `"ansi:*"` indices). Pairs a left-hand player and playlist stack with a toggleable right-hand 24-band FFT spectrum analyzer and Braille oscilloscope wing.

| Audio Source | Badge | Backend | Requirement |
| :--- | :--- | :--- | :--- |
| Media files / URLs | `FILE` | `QMediaPlayer` + `QAudioOutput` + `QAudioBufferOutput` | Qt Multimedia (FFmpeg backend) |
| Synthwave album | `DEMO` | Deterministic 5-track procedural synth in C++ (`--demo`) | None |

## Usage

```sh
./build/apps/audio_player/qmltui_audio_player [--demo] [--volume <0-100>] [media...]

./build/apps/audio_player/qmltui_audio_player ~/Music/album/          # directory (recursive, sorted)
./build/apps/audio_player/qmltui_audio_player track.flac other.mp3    # explicit playlist
./build/apps/audio_player/qmltui_audio_player https://host/stream.mp3 # stream URL
./build/apps/audio_player/qmltui_audio_player --demo                  # procedural synthwave album
```

## Controls

| Key | Action |
| :--- | :--- |
| `Space` / `C` | Pause or resume playback |
| `Z` / `X` / `C` / `V` / `B` | Previous (`Z`), Play (`X`), Pause (`C`), Stop (`V`), Next (`B`) |
| `N` / `P` | Next or previous track |
| `S` / `R` | Toggle Shuffle (`SHUF`) or Repeat (`REP`) |
| `L` | Open **Load / Add Media** dialog (file, directory, URL, or Demo Album) |
| `↑` / `↓` / `Enter` / `Del` | Move playlist cursor, play selected track (`Enter`), or remove track (`Del`) |
| `←` / `→` | Seek 5 s backward or forward |
| `+` / `-` | Adjust volume in 5 % steps |
| `,` / `.` | Adjust stereo balance 10 % left or right (click `BAL` to center) |
| `[` / `]` | Step oscilloscope low-pass filter preset |
| `T` | Toggle elapsed vs. remaining track clock |
| `K` | Cycle 16-color ANSI skin (`CLASSIC`, `AMBER AMP`, `CYBER NEON`) |
| `1` / `2` / `3` / `4` | Toggle Main Deck (`1`), Playlist (`2`), Spectrum (`3`), and Oscilloscope (`4`) |
| `W` | Cycle oscilloscope mode (`Stereo Scope`, `Phosphor Scope`, `Radial Scope`) |
| `A` | Toggle Braille cell color averaging on shared dots |
| `Q` / `Esc` | Close open dialog or quit application |
