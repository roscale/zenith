# Zenith

A Wayland compositor that uses Flutter for rendering.

## Features

- [x] Render top level surfaces
- [x] Render popups
- [x] Move windows
- [ ] Resize windows

## Dependencies

### Arch Linux

- `pacman -S base-devel unzip cmake ninja clang gtk3 wlroots`
- Download Flutter: https://docs.flutter.dev/get-started/install/linux#install-flutter-manually

Make sure `flutter` is in PATH.

### Any distribution

Install `make` and `docker`.

## Compiling

### Arch Linux

- cd into the project directory
- `./download_flutter_engine.sh` to download the precompiled platform-agnostic Flutter engine
- `flutter config --enable-linux-desktop` to enable the Linux desktop platform
- `flutter pub get` to download Flutter project dependencies

#### Debug

```
flutter build linux --debug
make debug_bundle -j6
```

#### Release

```
flutter build linux --release
make release_bundle -j6
```

The bundle is compiled at `build/zenith/[debug|release]/bundle/`.

### Any distribution

- cd into the project directory
- `make docker_build_image`
- `make docker_run_container`

#### Debug

`make docker_debug_bundle`

#### Release

`make docker_release_bundle`

## Running

Switch to another TTY.

```
cd build/zenith/[debug|release]/bundle/
./zenith
```

For development purposes you can also start Zenith from an existing X11 or Wayland compositor and it will show up as a
window.