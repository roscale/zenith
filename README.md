# Zenith

A wlroots-based Wayland compositor that uses Flutter for rendering.

Video demonstration: https://youtu.be/3Zh7Tk4oWo0

<p float="middle">
  <img src="screenshots/firefox.png" width="40%" />
  <img src="screenshots/task_switcher.png" width="40%" />
</p>

## Why?

- There are very few compositors for mobile devices compared to desktop.
- Most (all?) mobile compositors are still pretty young and generally unpolished.
- Flutter is becoming more and more popular among app developers, and it will significantly lower the bar for
  contributing to this project.
- Flutter significantly reduces development time, and lets you easily design beautiful user interfaces that compete
  with other platforms.

## Code navigation

- `lib/` Compositor Flutter code
- `src/` Display server C++ code
- `src/flutter_engine/` Flutter engine setup and method channel callbacks
- `src/util/` Useful classes and functions used in various places
- `src/third_party/` Imported files and libraries

## Run without compiling

### Mobian / Debian 12 Bookworm / Ubuntu 22.04 LTS

- Download the deb package from the [latest](https://github.com/roscale/zenith/releases/latest) release
- Install it with `sudo apt install ./package.deb`
- Read the [Running](#running) section

### Other distributions

- Download the bundle from the [latest](https://github.com/roscale/zenith/releases/latest) release
  version
- Install necessary dependencies by yourself. If you are on Arch Linux, run `install_dependencies_arch_linux.sh`
- Read the [Running](#running) section.

## Development dependencies

- Download Flutter: https://docs.flutter.dev/get-started/install/linux#install-flutter-manually
- Make sure `flutter` is in PATH
- On Debian or Ubuntu, run `install_dependencies_debian.sh`
- On Arch Linux, run `install_dependencies_arch_linux.sh`

## Building

- `flutter config --enable-linux-desktop` to enable the Linux desktop platform
- `flutter pub get` to download Flutter dependencies from pub.dev

#### Debug

```
make debug_bundle -j6
```

#### Profile

```
make profile_bundle -j6
```

#### Release

```
make release_bundle -j6
```

The bundle is compiled at `build/zenith/[debug|profile|release]/bundle/`.

If you use a Jetbrains IDE you should see multiple configurations for creating and running bundles instead of using the
command line.

## Running

Switch to another TTY.

Change directory where the `zenith` binary is. If you installed the deb package, it's in `/opt/zenith/`. If you built
the project using `make`, it's in `build/zenith/[debug|profile|release]/bundle/`.

Start the compositor:

```
./zenith [COMMAND]
```

Replace `COMMAND` with your program of choice to be launched with the compositor. The program must have Wayland
support (at the moment). Any QT or GTK app should work. You can launch a terminal emulator like `konsole`
or `terminator`, and use it to start subsequent programs. `gnome-terminal` is very picky and it seems to always choose
to connect to the Wayland socket of the host, so it may not work. If you want to launch Chromium or Chromium-based
apps, you must use `--enable-features=UseOzonePlatform --ozone-platform=wayland`.

Right now, only pin authentication is supported. Change your user password to a pin, otherwise you won't be able to
unlock the compositor.

If you want to start the compositor on your secondary screen, set the environment variable `ZENITH_OUTPUT=1`, `1` being
the
index of the output. The default choice is `0`, the primary screen.

For development purposes it is more convenient to start the compositor from an existing X11 or Wayland compositor, and
it will
show up as a window.

**Do not just run `flutter run`. It will not work. It will use the GTK embedder instead of the wlroots embedder.**

Press `Alt`+`Esc` to quit.

## Attaching a debugger

For C++, any debugger should work.

For Flutter, I suggest using VSCode. Run command "Attach to Flutter Process" and give it `http://127.0.0.1:12345/`. All
usual Flutter debugging and profiling tools should work just fine. If you want to attach the debugger without VSCode,
run `make attach_debugger`.

## Roadmap

You can see the roadmap in the `Projects` tab by clicking on the `Master's thesis` project. It is organized as a kanban
board, where each feature is a GitHub issue paired with a description and some additional commentary.

The roadmap will evolve over time. Tasks may be reordered, new features might be added or even removed depending on how
things are going.

# License

This project is licensed under GPL-2.0-or-later, see the LICENSE file for more information.