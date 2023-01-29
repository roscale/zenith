#pragma once

#include <cstddef>

template<typename T>
class array_view {
	T* ptr;
	size_t len;
public:
	array_view(T* ptr, size_t len) noexcept: ptr{ptr}, len{len} {}

	T& operator[](size_t i) noexcept {
		return ptr[i];
	}

	T const& operator[](size_t i) const noexcept {
		return ptr[i];
	}

	auto size() const noexcept {
		return len;
	}

	auto begin() noexcept {
		return ptr;
	}

	auto end() noexcept {
		return ptr + len;
	}
};
