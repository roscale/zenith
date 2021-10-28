cmake_minimum_required(VERSION 3.20)
project(flutter_embedder C)

set(CMAKE_C_STANDARD 11)

add_executable(flutter_embedder src/main.c src/embedder.h src/flutter_callbacks.c src/flutter_callbacks.h src/flutland_structs.h src/xdg_surface_callbacks.c src/xdg_surface_callbacks.h src/output_callbacks.c src/output_callbacks.h src/fix_y_flip.c src/fix_y_flip.h)
set(CMAKE_C_FLAGS "-lGL -lpthread -I/usr/include/libdrm -I/usr/include/pixman-1 -lwlroots  -lwayland-server  -lxkbcommon  -DWLR_USE_UNSTABLE -I ${PROJECT_SOURCE_DIR} -L ${PROJECT_SOURCE_DIR} -lflutter_engine")