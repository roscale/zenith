#include "defer.hpp"

#include <utility>

defer::defer(std::function<void()> f) : f(std::move(f)) {
}

defer::~defer() {
	f();
}
