#!/bin/bash

engine_revision=$(flutter --version | grep Engine | awk '{print $NF}')

curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-debug.zip >/tmp/elinux-x64-debug.zip
unzip -o /tmp/elinux-x64-debug.zip -d /tmp
mv /tmp/libflutter_engine.so libflutter_engine_debug.so

curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-profile.zip >/tmp/elinux-x64-profile.zip
unzip -o /tmp/elinux-x64-profile.zip -d /tmp
mv /tmp/libflutter_engine.so libflutter_engine_profile.so

curl -L https://github.com/sony/flutter-embedded-linux/releases/download/"$engine_revision"/elinux-x64-release.zip >/tmp/elinux-x64-release.zip
unzip -o /tmp/elinux-x64-release.zip -d /tmp
mv /tmp/libflutter_engine.so libflutter_engine_release.so
