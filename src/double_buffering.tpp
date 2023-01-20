#pragma once

template<class T>
T* DoubleBuffering<T>::start_rendering() {
	render_buffer = newest_buffer;
	return render_buffer.get();
}

template<class T>
void DoubleBuffering<T>::finish_rendering() {
	render_buffer = nullptr;
}

template<class T>
void DoubleBuffering<T>::commit_new_buffer(std::shared_ptr<T> buffer) {
	newest_buffer = std::move(buffer);
}

template<class T>
DoubleBuffering<T>::~DoubleBuffering() {
	render_buffer = nullptr;
	newest_buffer = nullptr;
}
