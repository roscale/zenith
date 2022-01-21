# Zenith

A wlroots-based Wayland compositor that uses Flutter for rendering.

![Vsync test](screenshots/vsync.png)

## Features and TODOs

- [x] Render top level surfaces (aka windows)
- [x] Render popups
- [x] Move windows
- [x] Resize windows
- [x] Milestone 1: Feature parity with tinywl
- [ ] Global kinetic scrolling using the touchpad
- [ ] Maximize windows
- [ ] XWayland
- [ ] Alt-tab switcher
- [ ] Server-side decorations
- [ ] Dock for open windows
- [ ] Minimize windows
- [ ] App launcher
- [ ] Quick settings
- [ ] Settings app
- [ ] Lockscreen
- [ ] Multi-monitor support. Might be higher on the list.
- [ ] Login manager

## Code navigation

- `lib/` Compositor Flutter code
- `src/` Display server C++ code
- `src/flutter_engine/` Flutter engine setup and method channel callbacks.
- `src/util/` Useful classes and functions used in various places
- `src/third_party/` Imported files and libraries

## Run without compilation

- Install `wlroots`
- Download the latest release
- Read the [Running](#running) section.

## Compilation dependencies

### Arch Linux

- `pacman -S base-devel git-lfs unzip cmake ninja clang gtk3 wlroots`
- Download Flutter: https://docs.flutter.dev/get-started/install/linux#install-flutter-manually

Make sure `flutter` is in PATH.

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

#### Profile

```
flutter build linux --profile
make profile_bundle -j6
```

#### Release

```
flutter build linux --release
make release_bundle -j6
```

The bundle is compiled at `build/zenith/[debug|profile|release]/bundle/`.

If you have problems compiling, have a look at the Github Action workflow and please fix this README by creating a pull
request.

If you use a Jetbrains IDE you should see multiple configurations for creating and running bundles instead of using the
command line.

## Running

Switch to another TTY.

```
cd build/zenith/[debug|profile|release]/bundle/
./zenith
```

For development purposes it is more convenient to start Zenith from an existing X11 or Wayland compositor, and it will
show up as a window.

**Do not just run `flutter run`. It will not work. It will use the GTK embedder instead of our beautiful C++ code.**

Press `Ctrl`+`Alt`+`Delete` to quit.

It's worth mentioning Flutter for Desktop is still in beta and thus, when running in debug mode you will see visual bugs
not present in profile or release mode. For some reason, in debug mode the loading of images doesn't seem to work and
windows will flicker sometimes. Let's hope these bugs will be fixed by the Flutter team. Compile in release mode if you
want a good experience.

## Attaching a debugger

For C++, any debugger should work.

For Flutter, I suggest using VSCode. Run command "Attach to Flutter Process" and give it `http://127.0.0.1:12345/`. All
usual Flutter debugging and profiling tools should work just fine. If you want to attach the debugger without VSCode,
run `make attach_debugger`.

## Screenshots

![Widget inspector](screenshots/widget_inspector.png)