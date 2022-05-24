#!/bin/bash

printf "\n* Installing dependencies *\n\n"

sudo apt install -y \
  ninja-build \
  libgtk-3-dev \
  cmake \
  clang \
  libwlroots-dev \
  libgl-dev \
  libegl-dev \
  libgles-dev \
  libepoxy-dev \
  libwayland-dev \
  libxkbcommon-dev \
  libpixman-1-dev \
  libinput-dev

printf "\n* Downloading the Flutter engine shared libraries *\n\n"

engine_revision=$(flutter --version | grep Engine | awk '{print $NF}')

curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-debug.zip >/tmp/elinux-x64-debug.zip
curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-profile.zip >/tmp/elinux-x64-profile.zip
curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-release.zip >/tmp/elinux-x64-release.zip

printf "\n* Extracting the Flutter engine libraries *\n\n"

mkdir deps

unzip -o /tmp/elinux-x64-debug.zip -d /tmp || exit
mv /tmp/libflutter_engine.so deps/libflutter_engine_debug.so

unzip -o /tmp/elinux-x64-profile.zip -d /tmp || exit
mv /tmp/libflutter_engine.so deps/libflutter_engine_profile.so

unzip -o /tmp/elinux-x64-release.zip -d /tmp || exit
mv /tmp/libflutter_engine.so deps/libflutter_engine_release.so

printf "\n* Done *\n\n"
