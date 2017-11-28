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
#include <crl/crl_dispatch_queue.h>

#include <dispatch/dispatch.h>

namespace crl {
namespace {

dispatch_queue_t Unwrap(void *value) {
	return static_cast<dispatch_queue_t>(value);
}

} // namespace

auto queue::implementation::create() -> pointer {
	auto result = dispatch_queue_create(nullptr, DISPATCH_QUEUE_SERIAL);
	if (!result) {
		throw std::bad_alloc();
	}
	return result;
}

void queue::implementation::operator()(pointer value) {
	if (value) {
		dispatch_release(Unwrap(value));
	}
};

queue::queue() : _handle(implementation::create()) {
}

void queue::async_plain(void (*callable)(void*), void *argument) {
	dispatch_async_f(
		Unwrap(_handle.get()),
		argument,
		callable);
}

void queue::sync_plain(void (*callable)(void*), void *argument) {
	dispatch_sync_f(
		Unwrap(_handle.get()),
		argument,
		callable);
}

queue::~queue() {
	dispatch_sync_f(Unwrap(_handle.get()), nullptr, [](void*) {});
	dispatch_release(Unwrap(_handle.get()));
}

} // namespace crl
