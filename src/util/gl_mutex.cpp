#include "gl_mutex.hpp"

void GLMutex::acquire() {
	if (sync != nullptr) {
		glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
	}
}

void GLMutex::release() {
	if (sync != nullptr) {
		glDeleteSync(sync);
	}
	sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

GLScopedLock::GLScopedLock(GLMutex& gl_mutex) : gl_mutex(gl_mutex) {
	gl_mutex.acquire();
}

GLScopedLock::~GLScopedLock() {
	gl_mutex.release();
}
