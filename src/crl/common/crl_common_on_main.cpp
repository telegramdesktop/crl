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
#include <crl/common/crl_common_on_main.h>

#if defined CRL_USE_COMMON_QUEUE || !defined CRL_USE_DISPATCH

namespace {

crl::queue *Queue/* = nullptr*/;
std::atomic<int> Counter/* = 0*/;
crl::details::main_queue_pointer Lifetime;

} // namespace

namespace crl::details {

void main_queue_pointer::grab() {
	auto counter = Counter.load(std::memory_order_acquire);
	while (true) {
		if (!counter) {
			return;
		} else if (Counter.compare_exchange_weak(counter, counter + 1)) {
			_pointer = Queue;
			return;
		}
	}
}

void main_queue_pointer::ungrab() {
	if (_pointer) {
		if (--Counter == 0) {
			delete _pointer;
		}
		_pointer = nullptr;
	}
}

void main_queue_pointer::create(queue_processor processor) {
	if (Counter.load(std::memory_order_acquire) != 0) {
		throw std::bad_alloc();
	}
	Queue = new queue(processor);
	Counter.store(1, std::memory_order_release);
	_pointer = Queue;
}

} // namespace crl::details

namespace crl {

void init_main_queue(queue_processor processor) {
	Lifetime.create(processor);
}

} // namespace crl

#endif // CRL_USE_COMMON_QUEUE || !CRL_USE_DISPATCH
