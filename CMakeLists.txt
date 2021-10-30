cmake_minimum_required(VERSION 3.20)
project(flutter_embedder)

set(CMAKE_CXX_STANDARD 17)

add_executable(flutter_embedder
        src/main.cpp
        src/embedder.h
        src/flutland_structs.hpp
        src/flutter_callbacks.cpp src/flutter_callbacks.hpp
        src/xdg_surface_callbacks.cpp src/xdg_surface_callbacks.hpp
        src/output_callbacks.cpp src/output_callbacks.hpp
        src/fix_y_flip.cpp src/fix_y_flip.hpp)

set(CMAKE_CXX_FLAGS "-lGL -lpthread -I/usr/include/libdrm -I/usr/include/pixman-1 -lwlroots  -lwayland-server  -lxkbcommon  -DWLR_USE_UNSTABLE -I ${PROJECT_SOURCE_DIR} -L ${PROJECT_SOURCE_DIR} -lflutter_engine")