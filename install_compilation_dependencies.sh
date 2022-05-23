#!/bin/bash

printf "\n* Installing dependencies *\n\n"

sudo pacman -S --needed "$@" \
  base-devel \
  clang \
  cmake \
  git \
  git-lfs \
  gtk3 \
  libdrm \
  libinput \
  libxcb \
  libxkbcommon \
  meson \
  opengl-driver \
  pixman \
  seatd \
  udev \
  unzip \
  wayland \
  wayland-protocols \
  wlroots \
  xcb-util-renderutil \
  xcb-util-wm \
  xorg-xwayland \
  zip

printf "\n* Downloading the Flutter engine shared libraries *\n\n"

engine_revision=$(flutter --version | grep Engine | awk '{print $NF}')

curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-debug.zip >/tmp/elinux-x64-debug.zip
curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-profile.zip >/tmp/elinux-x64-profile.zip
curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-release.zip >/tmp/elinux-x64-release.zip

printf "\n* Extracting the Flutter engine libraries *\n\n"

unzip -o /tmp/elinux-x64-debug.zip -d /tmp
mv /tmp/libflutter_engine.so deps/libflutter_engine_debug.so

unzip -o /tmp/elinux-x64-profile.zip -d /tmp
mv /tmp/libflutter_engine.so deps/libflutter_engine_profile.so

unzip -o /tmp/elinux-x64-release.zip -d /tmp
mv /tmp/libflutter_engine.so deps/libflutter_engine_release.so

printf "\n* Done *\n\n"
