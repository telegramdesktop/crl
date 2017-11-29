/*
This file is part of Telegram Desktop,
the official desktop version of Telegram messaging app, see https://telegram.org

Telegram Desktop is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

It is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

In addition, as a special exception, the copyright holders give permission
to link the code of portions of this program with the OpenSSL library.

Full license: https://github.com/telegramdesktop/tdesktop/blob/master/LICENSE
Copyright (c) 2014-2017 John Preston, https://desktop.telegram.org
*/
#pragma once

#include <crl/common/crl_common_config.h>
#include <memory>

#ifndef CRL_USE_DISPATCH
#error "This file should not be included by client-code directly."
#endif // CRL_USE_DISPATCH

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
		return *this;
	}

	void acquire();
	void release();

private:
	// Hide dispatch_semaphore_t
	struct implementation {
		using pointer = void*;
		static pointer create();
		void operator()(pointer value);
	};
	std::unique_ptr<implementation::pointer, implementation> _handle;

};

} // namespace crl
