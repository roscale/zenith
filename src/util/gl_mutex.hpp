#pragma once

#include <epoxy/gl.h>

/*
 * This implements mutual exclusion between 2 threads, on the GPU, using fences.
 * - This *DOES NOT* work for more than 2 threads!
 * - This *DOES NOT* block the CPU thread! It blocks the command queue related to the current context on the GPU.
 * - This class *IS NOT* thread safe on the CPU! Use std::mutex or other synchronization methods to synchronize
 * this class.
 */
class GLMutex {
private:
	GLsync sync = nullptr;

public:
	void acquire();

	void release();
};

// Nice to have. Analogous to std::scoped_lock.
class GLScopedLock {
private:
	GLMutex& gl_mutex;

public:
	explicit GLScopedLock(GLMutex& gl_mutex);

	~GLScopedLock();
};