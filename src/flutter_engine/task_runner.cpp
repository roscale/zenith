#include <iostream>
#include "task_runner.hpp"
#include <sys/timerfd.h>
#include <cassert>

TaskRunner::TaskRunner() : timer_fd{timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)} {
	expired_tasks.reserve(8);
}

void TaskRunner::set_engine(FlutterEngine engine) {
	this->engine = engine;
}

void TaskRunner::add_task(uint64_t target_time, FlutterTask task) {
	std::scoped_lock lock(mutex);
	tasks.emplace(target_time, task);
	schedule_timer();
}

void TaskRunner::execute_expired_tasks() {
	expired_tasks.clear();

	// We don't want to hold onto the mutex while executing tasks, slowing down other threads.
	// Collect all expired tasks before executing them.
	{
		std::scoped_lock lock(mutex);
		while (not tasks.empty() and FlutterEngineGetCurrentTime() >= tasks.top().first) {
			Task top_task = tasks.top();
			tasks.pop();
			expired_tasks.push_back(top_task);
		}
		schedule_timer();
	}

	for (Task& task: expired_tasks) {
		FlutterTask& flutter_task = task.second;
		FlutterEngineRunTask(engine, &flutter_task);
	}
}

void TaskRunner::schedule_timer() const {
	if (tasks.empty()) {
		return;
	}

	long next_schedule_s;
	long next_schedule_ns;

	uint64_t first_task_target_time = tasks.top().first;
	uint64_t now = FlutterEngineGetCurrentTime();

	if (now >= first_task_target_time) {
		// The first task should already execute. Schedule the task runner ASAP.
		next_schedule_s = 0;
		next_schedule_ns = 1;
	} else {
		uint64_t diff = first_task_target_time - now;
		long ns_until_first_task = (long) diff;
		assert(ns_until_first_task > 0);

		next_schedule_s = ns_until_first_task / 1'000'000'000;
		next_schedule_ns = ns_until_first_task % 1'000'000'000;
	}

	itimerspec new_value = {
		  .it_interval = {.tv_sec = 0, .tv_nsec = 0},
		  .it_value = {
				.tv_sec = next_schedule_s,
				.tv_nsec = next_schedule_ns,
		  },
	};
	timerfd_settime(timer_fd, 0, &new_value, nullptr);
}

bool task_runner_compare::operator()(const Task& t1, const Task& t2) const {
	return t1.first > t2.first;
}
