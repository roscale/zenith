#include "time.hpp"
#include "embedder.h"

uint64_t current_time_nanoseconds() {
	return FlutterEngineGetCurrentTime();
}

uint64_t current_time_microseconds() {
	return FlutterEngineGetCurrentTime() / 1'000;
}

uint32_t current_time_milliseconds() {
	return FlutterEngineGetCurrentTime() / 1'000'000;
}
