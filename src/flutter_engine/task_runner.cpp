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

uint64_t TaskRunner::execute_expired_tasks() {
	std::list<Task> expired_tasks{};

	mutex.lock();
	while (not tasks.empty() and FlutterEngineGetCurrentTime() >= tasks.top().first) {
		Task top_task = tasks.top();
		tasks.pop();
		expired_tasks.push_back(top_task);
	}
	mutex.unlock();

	for (Task& task: expired_tasks) {
		FlutterTask& flutter_task = task.second;
		FlutterEngineRunTask(engine, &flutter_task);
	}

	mutex.lock();
	if (tasks.empty()) {
		mutex.unlock();
		// Reschedule in 1ms.
		return FlutterEngineGetCurrentTime() + 1'000'000;
	}

	Task top_task = tasks.top();
	mutex.unlock();
	// Reschedule to the time of the earliest task.
	return top_task.first;
}
