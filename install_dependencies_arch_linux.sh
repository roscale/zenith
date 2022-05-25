#!/bin/bash

sudo pacman -S --needed --noconfirm \
  base-devel \
  ninja \
  gtk3 \
  cmake \
  clang \
  wlroots \
  libglvnd \
  libepoxy \
  wayland \
  libxkbcommon \
  pixman \
  libinput
