cmake_minimum_required(VERSION 3.20)
project(flutter_embedder)

set(CMAKE_C_STANDARD 11)

add_executable(flutter_embedder main.c)
set (CMAKE_C_FLAGS "-lGL -I/usr/include/libdrm -I/usr/include/pixman-1 -lwlroots  -lwayland-server  -lxkbcommon  -DWLR_USE_UNSTABLE -I ${PROJECT_SOURCE_DIR} -L ${PROJECT_SOURCE_DIR} -lflutter_engine")