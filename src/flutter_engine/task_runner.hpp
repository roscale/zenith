#pragma once

#include "embedder.h"
#include <queue>
#include <mutex>
#include <list>

using Task = std::pair<uint64_t, FlutterTask>;

struct task_runner_compare {
	bool operator()(const Task& t1, const Task& t2) const;
};

class TaskRunner {
	FlutterEngine engine;
	std::mutex mutex{};

public:
	std::priority_queue<Task, std::vector<Task>, task_runner_compare> tasks{};
	void set_engine(FlutterEngine engine);

	void add_task(uint64_t target_time, FlutterTask task);

	void execute_expired_tasks();
};
