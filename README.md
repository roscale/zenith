# Zenith

A wlroots-based Wayland compositor that uses Flutter for rendering.

![Firefox](screenshots/firefox.png)
![Task switcher](screenshots/task_switcher.png)

## Code navigation

- `lib/` Compositor Flutter code
- `src/` Display server C++ code
- `src/flutter_engine/` Flutter engine setup and method channel callbacks.
- `src/util/` Useful classes and functions used in various places
- `src/third_party/` Imported files and libraries

## Run without compilation

### Arch Linux

- `./install_runtime_dependencies.sh`
- Download the [latest](https://github.com/roscale/zenith/releases/latest) release
- Read the [Running](#running) section.

## Compilation dependencies

### Arch Linux

- Download Flutter: https://docs.flutter.dev/get-started/install/linux#install-flutter-manually
- `./install_compilation_dependencies.sh`

Make sure `flutter` is in PATH.

## Compiling

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

**Do not just run `flutter run`. It will not work. It will use the GTK embedder instead of our wlroots embedder.**

Press `Ctrl`+`Alt`+`Delete` to quit.

## Attaching a debugger

For C++, any debugger should work.

For Flutter, I suggest using VSCode. Run command "Attach to Flutter Process" and give it `http://127.0.0.1:12345/`. All
usual Flutter debugging and profiling tools should work just fine. If you want to attach the debugger without VSCode,
run `make attach_debugger`.

## Roadmap

Roadmap is most likely going to change but here's a list of things I want to implement paired with some commentary.

- [x] [Set up the embedder and the Flutter Engine](#set-up-the-embedder-and-the-flutter-engine)
- [x] [Display toplevel and popup surfaces on the screen](#display-toplevel-and-popup-surfaces-on-the-screen)
- [ ] [Touch input](#touch-input)
- [ ] [Task switcher](#task-switcher)
    - [x] Manually switch between tasks
    - [x] Animate when an app is opened or closed
    - [ ] Flick up to close a task
- [ ] [Virtual keyboard](#virtual-keyboard)
- [ ] [Desktop file parser](#desktop-file-parser)
- [ ] [App drawer](#app-drawer)
- [ ] [Settings app](#settings-app)
    - [ ] Connect to Wi-Fi
    - [ ] Change sound volume
- [ ] [Status bar and quick settings](#status-bar-and-quick-settings)
    - [ ] Show clock
    - [ ] Show Wi-Fi signal strength
    - [ ] Show battery level
    - [ ] Turn on/off Wi-Fi
    - [ ] Change screen brightness
- [ ] [Power buttons](#power-buttons)
- [ ] [Login manager](#login-manager)
- [ ] [Lock screen](#lock-screen)
- [ ] [Write or adopt a set of mobile friendly core apps](#write-or-adopt-a-set-of-mobile-friendly-core-apps)
- [ ] [Notifications](#notifications)
- [ ] [MPRIS](#mpris)
- [ ] [Rotate to landscape in fullscreen apps](#rotate-to-landscape-in-fullscreen-apps)
- [ ] [Home screen](#home-screen)
- [ ] [Screenshotting](#screenshotting)
- [ ] [Built-in screen recording](#built-in-screen-recording)
- [ ] [Integrate Android apps using Waydroid](#integrate-android-apps-using-waydroid)
- [ ] [Customization](#customization)
- [ ] [Home screen widgets](#home-screen-widgets)
- [ ] [Autorotate](#autorotate)
- [ ] [Localization and internationalization](#localization-and-internationalization)
- [ ] [Integrate XWayland](#integrate-xwayland)

### Set up the embedder and the Flutter Engine

Make sure the embedder can receive Wayland clients. Set up the Flutter Engine and render something on the screen.

### Display toplevel and popup surfaces on the screen

Copy surface textures of Wayland clients and register them to the Flutter Engine using their OpenGL handle. Use
the `Texture` widget to display the textures to the screen. Use the C++ library from Google to encode and decode
platform messages to be able to send and receive messages from Dart. Notify Flutter though platform channels when a new
window or popup appears or disappears. Construct a tree of windows and popups using `Stack`. Popups always need to be
positioned somewhere relative to its parent, whether it is a window or another popup.

Pointer support is implemented for testing on desktop. Pointer events are routed to the Flutter Engine. Using
a `GestureDetector` on each surface, we can know to which application we need to forward this event with what
coordinates relative to the surface. The event is routed back to the embedder which will then be forwarded to the right
application.

### Touch input

The DE and Wayland clients are able to receive touch input.

Right now, only Flutter captures touch input, not the clients. Flutter should get all touch events and should respond
back to the display server if the touch event is for a client. Only Flutter knows where the windows and popups are on
the screen, and the coordinates of touch events relative to the client's surface.

### Task switcher

Have a working task switcher similar to Android. The user is able to scroll through open apps and choose one to show
fullscreen. By flicking up, the user can close an app and remove it from the task switcher.

Right now, the task switcher is mostly working, but you cannot close an app with an upward flick. Let's not forget that
on the desktop platform, some apps will not close immediately after requesting a close request via Wayland. Some will
spawn a popup if you have unsaved changes.

### Virtual keyboard

The user is able to show and hide the keyboard using a button on the screen. At least the English layout will be
available.

The virtual keyboard should automatically appear when the user taps on a text field but this is dependent on whether the
app's toolkit implements this Wayland protocol: https://wayland.app/protocols/text-input-unstable-v3. It might be worth
implementing it inside the desktop environment if GTK, Qt, and Chromium-based apps use it.

The virtual keyboard will talk to the display server though a platform channel when a key is pressed, and the display
server will emulate the key for the currently focused surface.

### Desktop file parser

Write a parser in Dart for `.desktop` files and be able to obtain a list of all installed applications with their name,
search keywords, icon, and of course the location of the executable.

Desktop entry files have a public specification that I will need to read and implement:
https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html

### App drawer

The user is able to scroll and search though a list of all installed apps on the system. He can run an app by
tapping on its icon which will beautifully animate the app opening. He no longer has to use the terminal to start
apps.

The app launcher and the task switcher will probably be very linked one another because task widgets will have to be
animated when an app is launched, and when it is closed, like on Android.

### Settings app

The user can change network, audio, and brightness settings by using the Settings app. At a minimum, the user should be
able to see nearby Wi-Fi networks and connect to them if they are password-less or if they use WPA2. He will also be
able to adjust the master volume.

The Settings app will be written in Flutter and will use D-Bus to communicate to various system services.
There's already a D-Bus library for Dart written by Canonical. Fun fact, the installer for Ubuntu 22.04 is a Flutter
app, so I think this is the reason they wrote this library.
https://pub.dev/packages/dbus

I will have to read the D-Bus API documentation of various system services: NetworkManager, ModemManager, PipeWire (or
PulseAudio), and Bluez.

Exposing the complete API for these services in the Settings app could take some time. If time allows I will add support
for mobile data, bluetooth pairing, and other network settings.

### Status bar and quick settings

The user will be able to see the battery level, and the clock.
Retrieve the battery level and use the code written previously to access network connectivity status, and show this
information on the status bar.

Using quick settings, the user should be able to toggle on or off network connectivity, screen brightness, and other
parameters.

At a minimum, the screen brightness should be adjustable.

### Power buttons

Have buttons for shutdown and restart.

Mobile devices allow the operating system to detect long presses on the power button. The firmware of desktop devices
however, is programmed to halt the system without notifying the operating system.

Using the systemd-logind D-Bus interface, we can shut down and restart the machine without superuser privileges.

https://www.freedesktop.org/software/systemd/man/org.freedesktop.login1.html

### Login manager

When the device is powered on, the user will be able to log in using a pattern.

The desktop environment will also have to play the role of login manager and authenticate the user using PAM. If the
credentials are correct, a session is created, the desktop environment will downgrade its privileges from root to
user, and it will put the user on the home screen. This unified architecture of login screen and DE will let me
achieve a nearly instant and flicker-free login experience.

Translating a pattern to a password should be easy. If time allows, I will implement other login methods: pin and
password.

I have a feeling I will have to write a PAM configuration file, a systemd service file, and when the user is
authenticated, D-Bus, NetworkManager, PulseAudio, and other services will have to be started with the help of systemd.

https://www.freedesktop.org/wiki/Software/systemd/writing-desktop-environments/
https://www.freedesktop.org/software/systemd/man/org.freedesktop.login1.html
https://www.man7.org/linux/man-pages/man8/pam_systemd.8.html

This is the point where the desktop environment will be able to start on its own without having other desktop
environments installed.

### Lock screen

When the user presses the power button, the screen turns off. When the user presses the power button again, the
screen turns on and displays a lock screen asking the user to authenticate.

Once again, PAM will probably come in handy to verify if the password is valid or not.

Turning the screen off on OLED screens could be simulated by simply filling the entire screen with the color black. The
screen could also be turned off for real if we want it to work with regular LED screens, but it might introduce
additional latency when turning the screen back on, and I don't want that. VSync signals should be ignored and Flutter
should not generate new frames until the user presses again the power button.

### Write or adopt a set of mobile friendly core apps

File manager, notes app, calendar, calculator, camera, etc. are all applications that should exist. If you install Phosh
or Plasma Mobile, you already have some preinstalled apps. Maybe the easiest would be to adopt already existing apps,
but we would lose design consistency. If we go this route it would be best if we can find some Flatpak apps that
integrate well.

A store app for installing Flatpak apps from Flathub without elevated privileges would be nice to have. Once again,
remains to be seen if we should adopt an already existing app or if we should write our own.

If time allows, I could write some of these apps in Flutter. Ideally, we would host our own Flatpak repository with our
own apps known to work well on a mobile device.

### Notifications

The user should get notifications from applications. These notifications should be visible by opening quick settings.

All apps send notifications to the desktop environment via D-Bus. The desktop environment must expose a D-Bus object on
some standard path.

https://specifications.freedesktop.org/notification-spec/notification-spec-latest.html

### MPRIS

The user can control media player applications directly from quick settings. If he uses the browser to watch a YouTube
video or to listen to music on Spotify, he can play, pause, and possibly scrub the media from outside the app in the
quick settings, or even on the lock screen.

https://specifications.freedesktop.org/mpris-spec/latest/

### Rotate to landscape in fullscreen apps

When the user wants to watch a video on YouTube in the browser, it will automatically be shown in landscape mode.

When any surface requests fullscreen, it will be rotated in landscape mode, and touch input will obviously be
transformed to match the rotation.

### Home screen

The user will be able to take any app from the drawer and arrange it on the home screen to its liking. He will be able
to put his apps on multiple pages in a grid.

### Screenshotting

The user must be able to take screenshots.

This should be fairly easy to implement by just copying the framebuffer texture, and then saving it as a PNG image using
an PNG encoder.

### Built-in screen recording

The user must be able to record its screen and save it as a video.

Again, once we copy the framebuffer texture, we just pass that to a video encoder which will do the job of encoding the
frames to an MP4 file.

### Integrate Android apps using Waydroid

Waydroid runs LineageOS in a container and lets you run Android apps each one in its own window.

It would be huge if we could seamlessly integrate Android apps into this. Of course, we cannot preinstall Google apps
and services due to legal reasons, but having the possibility of running Android apps is a big plus.

### Customization

The user can change the accent color of the desktop environment, switch to dark theme, change the wallpaper on the
login/lock screen and on the login screen, etc.

### Home screen widgets

The user can place widgets on the home screen, like an analog clock or a weather widget.

### Autorotate

The user can enable screen autorotation and put the device in landscape mode.

This feature could be a problem to implement if the rotation sensor is not accessible. Are all sensors exposed though a
common interface?

### Localization and internationalization

Translate the desktop environment in multiple languages, add multiple virtual keyboard layouts.

### Integrate XWayland

XWayland integration would allow for running X11 apps. Mobile friendly Wayland apps are few, mobile friendly X11 apps
are even fewer (or non-existent?). It would be nice to have for compatibility though.