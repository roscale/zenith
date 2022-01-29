#pragma once

#include <functional>

/*
 * Analogous to the "defer" keyword in Golang, but a bit uglier.
 */
class defer {
	std::function<void()> f;
public:
	explicit defer(std::function<void()> lambda);

	~defer();
};