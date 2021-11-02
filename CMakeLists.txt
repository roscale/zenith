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
        src/fix_y_flip.cpp src/fix_y_flip.hpp
        src/platform_channels/binary_messenger.cpp src/platform_channels/binary_messenger.hpp
        src/platform_channels/incoming_message_dispatcher.cpp src/platform_channels/incoming_message_dispatcher.hpp
        src/platform_channels/engine_method_result.h
        src/platform_channels/event_channel.h
        src/platform_channels/event_sink.h
        src/platform_channels/event_stream_handler.h
        src/platform_channels/standard_codec.cc
        src/new_texture_stream_handler.hpp
        src/input_callbacks.hpp src/input_callbacks.cpp src/create_shared_egl_context.cpp src/create_shared_egl_context.hpp)

target_include_directories(flutter_embedder PRIVATE src/third_party/rapidjson/include)

set(CMAKE_CXX_FLAGS "-g -lGL -lEGL -lGLESv2 -lpthread -I/usr/include/libdrm -I/usr/include/pixman-1 -lwlroots  -lwayland-server  -lxkbcommon  -DWLR_USE_UNSTABLE -I ${PROJECT_SOURCE_DIR} -L ${PROJECT_SOURCE_DIR} -lflutter_engine")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")