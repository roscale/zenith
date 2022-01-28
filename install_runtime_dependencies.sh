#!/bin/bash

sudo pacman -S --needed "$@" \
  expat \
  gcc-libs \
  glib2 \
  glibc \
  libcap \
  libdrm \
  libepoxy \
  libevdev \
  libffi \
  libgcrypt \
  libglvnd \
  libgpg-error \
  libgudev \
  libinput \
  libwacom \
  libx11 \
  libxau \
  libxcb \
  libxdmcp \
  libxkbcommon \
  lz4 \
  mtdev \
  opengl-driver \
  pcre \
  pixman \
  seatd \
  systemd-libs \
  wayland \
  xcb-util-renderutil \
  xcb-util-wm \
  xz \
  zstd
