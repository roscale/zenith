# Zenith

A wlroots-based Wayland compositor that uses Flutter for rendering.

![Vsync test](screenshots/vsync.png)

## Features and TODOs

- [x] Render top level surfaces (aka windows)
- [x] Render popups
- [x] Move windows
- [x] Resize windows
- [x] Milestone 1: Feature parity with tinywl
- [ ] Global kinetic scrolling when using the touchpad
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
- Download the [latest](https://github.com/roscale/zenith/releases/latest) release
- Read the [Running](#running) section.

## Compilation dependencies

### Arch Linux

- `pacman -S base-devel git-lfs unzip cmake ninja meson clang gtk3 wlroots wayland-protocols xorg-xwayland`
- Download Flutter: https://docs.flutter.dev/get-started/install/linux#install-flutter-manually

Make sure `flutter` is in PATH.

## Compiling

### Arch Linux

- cd into the project directory
- `./download_dependencies.sh` to download and compile the required libraries
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
./zenith COMMAND
```

Replace `COMMAND` with your program of choice to be launched with the compositor. The program must have Wayland
support (at the moment). Any QT or GTK app should work. You can launch a terminal emulator like `konsole`
or `gnome-terminal`, and use it to start subsequent programs. If you want to launch Chromium, you must
use `--enable-features=UseOzonePlatform --ozone-platform=wayland`.

If you want to start Zenith on your secondary screen, set the environment variable `ZENITH_OUTPUT=1`, `1` being the
index of the output. The default choice is `0`, the primary screen. This setting is temporary until multi-monitor
support is implemented.

For development purposes it is more convenient to start Zenith from an existing X11 or Wayland compositor, and it will
show up as a window.

**Do not just run `flutter run`. It will not work. It will use the GTK embedder instead of our beautiful C++ code.**

Press `Ctrl`+`Alt`+`Delete` to quit.

It's worth mentioning Flutter for Desktop is still in beta and thus, when running in debug mode you will see visual bugs
not present in profile or release mode. For some reason, in debug mode the loading of images doesn't seem to work and
windows will flicker sometimes. I'm almost certain these bugs come from the Flutter engine so let's hope they will be
fixed soon. Compile in release mode if you want a good experience.

## Attaching a debugger

For C++, any debugger should work.

For Flutter, I suggest using VSCode. Run command "Attach to Flutter Process" and give it `http://127.0.0.1:12345/`. All
usual Flutter debugging and profiling tools should work just fine. If you want to attach the debugger without VSCode,
run `make attach_debugger`.

## Screenshots

![Widget inspector](screenshots/widget_inspector.png)