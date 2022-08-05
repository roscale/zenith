#pragma once

#include <ostream>

struct Offset {
	double dx, dy;

	friend std::ostream& operator<<(std::ostream& os, const Offset& offset) {
		os << "Offset{dx: " << offset.dx << " dy: " << offset.dy << "}";
		return os;
	}
};

struct Size {
	uint32_t width, height;

	friend std::ostream& operator<<(std::ostream& os, const Size& size) {
		os << "Size{width: " << size.width << " height: " << size.height << "}";
		return os;
	}
};