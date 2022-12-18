#pragma once

#include "output.hpp"

void activate_window(ZenithServer* server,
                     const flutter::MethodCall<>& call,
                     std::unique_ptr<flutter::MethodResult<>>&& result);

void pointer_hover(ZenithServer* server,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result);

void pointer_exit(ZenithServer* server,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result);

void close_window(ZenithServer* server,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result);

void resize_window(ZenithServer* server,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result);

// We want to keep the last frame of a window or popup in order to animate the closing.
// When the closing animation is done, there's no reason to keep the texture on the GPU and leak VRAM.
void unregister_view_texture(ZenithServer* server,
                             const flutter::MethodCall<>& call,
                             std::unique_ptr<flutter::MethodResult<>>&& result);

void mouse_button_event(ZenithServer* server,
                        const flutter::MethodCall<>& call,
                        std::unique_ptr<flutter::MethodResult<>>&& result);

void change_window_visibility(ZenithServer* server,
                              const flutter::MethodCall<>& call,
                              std::unique_ptr<flutter::MethodResult<>>&& result);

void touch_down(ZenithServer* server,
                const flutter::MethodCall<>& call,
                std::unique_ptr<flutter::MethodResult<>>&& result);

void touch_motion(ZenithServer* server,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result);

void touch_up(ZenithServer* server,
              const flutter::MethodCall<>& call,
              std::unique_ptr<flutter::MethodResult<>>&& result);

void touch_cancel(ZenithServer* server,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result);

void insert_text(ZenithServer* server,
                 const flutter::MethodCall<>& call,
                 std::unique_ptr<flutter::MethodResult<>>&& result);

void emulate_keycode(ZenithServer* server,
                     const flutter::MethodCall<>& call,
                     std::unique_ptr<flutter::MethodResult<>>&& result);

void initial_window_size(ZenithServer* server,
                         const flutter::MethodCall<>& call,
                         std::unique_ptr<flutter::MethodResult<>>&& result);

void startup_complete(ZenithServer* server,
                      const flutter::MethodCall<>& call,
                      std::unique_ptr<flutter::MethodResult<>>&& result);

void unlock_session(ZenithServer* server,
                    const flutter::MethodCall<>& call,
                    std::unique_ptr<flutter::MethodResult<>>&& result);
