#!/bin/bash

sudo apt update -y
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
