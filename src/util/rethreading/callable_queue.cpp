#include "callable_queue.hpp"

int CallableQueue::get_fd() const {
	return fd;
}

void CallableQueue::enqueue(const CallableQueue::Callable& callable) {
	std::scoped_lock lock(mutex);
	callables.push(callable);
	eventfd_write(fd, 1);
}

bool CallableQueue::execute() {
	std::queue<Callable> processing;
	{
		std::scoped_lock lock(mutex);
		eventfd_t value;
		if (eventfd_read(fd, &value) == -1) {
			return false;
		}

		processing = std::move(callables);
		callables = {};
	}

	while (!processing.empty()) {
		Callable callable = std::move(processing.front());
		processing.pop();
		callable();
	}
	return true;
}
