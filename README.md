# StealthDiva

A stealth-friendly mod for Hatsune Miku: Project DIVA Mega Mix+. Keep the rhythm game, lose the distractions.

> **A newer mod that supersedes DivaDim.** StealthDiva is based on [mrcloverthecoder/divadim](https://github.com/mrcloverthecoder/divadim) (credit to its author for the original PV dimming idea and code). It keeps everything DivaDim does and adds new functions: support for the early-2023 game build, stepped dimming hotkeys, scene skip to cut GPU load and save battery(for mobile device), and loading background hiding.

- Hides or dims the PV (music video), so you only see the notes.
  - `-` / `=` darken or brighten the overlay, 10% per press by default (`hotkey_step`). Works in the pause menu too.
  - `default_dim` and `pv_settings` in `config.toml` set the starting level, globally or per PV.
  - At 100% dim the scene is always skipped and `0` does nothing; below 100% `0` works again.
  - `0` toggles scene skip (it never changes the dim level): stage, movie, characters and Auth3d stop drawing, which cuts GPU load and power. Set `hotkey_stealth` (virtual-key code) in `config.toml`. The toggle state (`stealth`) and the dim level (`default_dim`) are saved automatically and restored next launch. Particles and some effects still draw.
  - `hide_loading_bg` (default true) hides the song loading background image.

It is a [DIVA Mod Loader](https://github.com/blueskythlikesclouds/DivaModLoader) DLL mod and works on the current Steam build and the early 2023 build.

## Install

Copy the `StealthDiva` folder from `release/` into the game's `mods/` folder.

Steam Deck: set the launch option `WINEDLLOVERRIDES='dinput8.dll=n,b' %command%`.


## Build from source

Run `build.cmd` from a VS x64 developer prompt. It builds with the static runtime (so it loads under Proton) and refreshes `release/StealthDiva/` with the DLL and `config.toml`.

Support for a new game version: see [DECODING_NEW_EXE.md](docs/DECODING_NEW_EXE.md).
