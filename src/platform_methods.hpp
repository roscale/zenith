#pragma once

#include "flutland_structs.hpp"

void activate_window(FlutlandOutput* output,
                     const flutter::MethodCall<> &call,
                     std::unique_ptr<flutter::MethodResult<>> result);

void pointer_hover(FlutlandOutput* output,
                   const flutter::MethodCall<> &call,
                   std::unique_ptr<flutter::MethodResult<>> result);

void close_window(FlutlandOutput* output,
                  const flutter::MethodCall<> &call,
                  std::unique_ptr<flutter::MethodResult<>> result);