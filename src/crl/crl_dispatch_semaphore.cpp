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
#include <crl/crl_dispatch_semaphore.h>

#include <dispatch/dispatch.h>

namespace crl {
namespace {

dispatch_semaphore_t Unwrap(void *value) {
	return static_cast<dispatch_semaphore_t>(value);
}

} // namespace

auto semaphore::implementation::create() -> pointer {
	auto result = dispatch_semaphore_create(0);
	if (!result) {
		throw std::bad_alloc();
	}
	return result;
}

void semaphore::implementation::operator()(pointer value) {
	if (value) {
		dispatch_release(Unwrap(value));
	}
};

void semaphore::acquire() {
	dispatch_semaphore_wait(Unwrap(_handle.get()), DISPATCH_TIME_FOREVER);
}

void semaphore::release() {
	dispatch_semaphore_signal(Unwrap(_handle.get()));
}

} // namespace crl
