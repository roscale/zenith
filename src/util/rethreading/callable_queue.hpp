#pragma once

#include <functional>
#include <queue>
#include "channel.hpp"
#include <iostream>
#include "assert.hpp"
#include <sys/eventfd.h>
#include <optional>

/*
 * Queue that stores and executes functions.
 * An event loop using epoll or other Linux APIs can listen the readability of `fd`.
 * When `fd` becomes readable, the event loop can call `execute` which will execute all
 * stored functions.
 *
 * This is useful for executing some tasks on another thread that has an event loop listening
 * to file descriptors.
 * It is mainly used to execute tasks on the main thread which has a `wl_event_loop`.
 */
class CallableQueue {
	using Callable = std::function<void(void)>;

	int fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	std::queue<Callable> callables = {};
	std::mutex mutex;

public:
	[[nodiscard]] int get_fd() const;

	void enqueue(const Callable& callable);

	bool execute();
};
