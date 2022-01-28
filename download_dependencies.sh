#!/bin/bash

engine_revision=$(flutter --version | grep Engine | awk '{print $NF}')

printf "\n* Downloading the Flutter engine shared libraries *\n\n"

curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-debug.zip >/tmp/elinux-x64-debug.zip
curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-profile.zip >/tmp/elinux-x64-profile.zip
curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-release.zip >/tmp/elinux-x64-release.zip

printf "\n* Downloading wlroots source code *\n\n"

mkdir deps
git clone -b 0.14.1 https://gitlab.freedesktop.org/wlroots/wlroots.git deps/wlroots

printf "\n* Extracting the Flutter engine libraries *\n\n"

unzip -o /tmp/elinux-x64-debug.zip -d /tmp
mv /tmp/libflutter_engine.so deps/libflutter_engine_debug.so

unzip -o /tmp/elinux-x64-profile.zip -d /tmp
mv /tmp/libflutter_engine.so deps/libflutter_engine_profile.so

unzip -o /tmp/elinux-x64-release.zip -d /tmp
mv /tmp/libflutter_engine.so deps/libflutter_engine_release.so

printf "\n* Compiling wlroots *\n\n"

cd deps/wlroots || exit
meson -Dxcb-errors=disabled build/
ninja -C build/

printf "\n* Done *\n\n"
