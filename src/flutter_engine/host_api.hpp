#pragma once

#include "output.hpp"

void activate_window(ZenithOutput* output,
                     const flutter::MethodCall<>& call,
                     std::unique_ptr<flutter::MethodResult<>>&& result);

void pointer_hover(ZenithOutput* output,
                   const flutter::MethodCall<>& call,
                   std::unique_ptr<flutter::MethodResult<>>&& result);

void pointer_exit(ZenithOutput* output,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result);

void close_window(ZenithOutput* output,
                  const flutter::MethodCall<>& call,
                  std::unique_ptr<flutter::MethodResult<>>&& result);