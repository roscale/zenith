# Zenith

A Wayland compositor that uses Flutter for rendering.

## Features

- [x] Render top level surfaces
- [x] Render popups
- [x] Move windows
- [ ] Resize windows

## Dependencies

### Arch Linux

Install Flutter, `curl`, `unzip`, `make`, `wayland`, `wayland-protocols`.

### Ubuntu

Install Flutter

`sudo apt install curl unzip make g++ libegl-dev libwlroots-dev `

Make sure `flutter` is in PATH.

## Compiling

- cd into the project directory
- `./download_flutter_engine.sh` to download the precompiled flutter engine
- `flutter config --enable-linux-desktop`

### Debug

```
flutter build linux --debug
make debug_bundle -j6
```

### Release

```
flutter build linux --release
make release_bundle -j6
```

The bundle is compiled at `build/zenith/[debug|release]/bundle/`.

## Running

Switch to another TTY.

```
cd build/zenith/[debug|release]/bundle/
./zenith
```

For development purposes you can also start Zenith in an existing X11 or Wayland compositor and it will show up as a
window.