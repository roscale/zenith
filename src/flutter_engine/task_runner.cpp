#include <EGL/egl.h>
#include <cassert>
#include "task_runner.hpp"

bool compare::operator()(const Task& t1, const Task& t2) {
	return t1.first < t2.first;
}

void TaskRunner::set_engine(FlutterEngine engine) {
	this->engine = engine;
}

void TaskRunner::add_task(uint64_t target_time, FlutterTask task) {
	mutex.lock();
	tasks.push(std::make_pair(target_time, task));
	mutex.unlock();
}

void TaskRunner::execute_expired_tasks() {
	std::list<Task> expired_tasks{};

	// We don't want to hold onto the mutex while executing tasks, slowing down other threads.
	// Collect all expired tasks before executing them.
	mutex.lock();
	while (not tasks.empty() and FlutterEngineGetCurrentTime() >= tasks.top().first) {
		Task top_task = tasks.top();
		tasks.pop();
		expired_tasks.push_back(top_task);
	}
	mutex.unlock();


	for (Task& task: expired_tasks) {
		FlutterTask& flutter_task = task.second;
//		assert(eglGetCurrentContext() != EGL_NO_CONTEXT);
		FlutterEngineRunTask(engine, &flutter_task);
	}

	// This task runner will be rescheduled every 1ms.
}
