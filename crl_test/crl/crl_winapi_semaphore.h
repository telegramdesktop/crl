#pragma once

#include <memory>

namespace crl {

class semaphore {
public:
	semaphore() : _handle(implementation::create()) {
	}
	semaphore(const semaphore &other) = delete;
	semaphore &operator=(const semaphore &other) = delete;
	semaphore(semaphore &&other) noexcept
	: _handle(std::move(other._handle)) {
	}
	semaphore &operator=(semaphore &&other) noexcept {
		_handle = std::move(other._handle);
	}

	void acquire();
	void release();

private:
	// Hide WinAPI HANDLE
	struct implementation {
		using pointer = void*;
		static pointer create();
		void operator()(pointer value);
	};
	std::unique_ptr<implementation::pointer, implementation> _handle;

};

} // namespace crl
