FROM archlinux:base-devel

RUN pacman --noconfirm -Syu unzip git cmake ninja clang gtk3 wlroots rsync

WORKDIR "/home"

RUN git clone https://github.com/flutter/flutter.git -b stable
ENV PATH="/home/flutter/bin:${PATH}"
RUN flutter precache --linux

COPY download_flutter_engine.sh .
RUN ./download_flutter_engine.sh