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
#include <crl/common/crl_common_queue.h>

#ifndef CRL_USE_DISPATCH

#include <crl/crl_async.h>

namespace crl {

queue::queue() : queue(&details::async_plain) {
}

queue::queue(ProcessCaller caller)
: _caller(caller) {
}

void queue::wake_async() {
	auto expected = false;
	if (_queued.compare_exchange_strong(expected, true)) {
		_caller(
			ProcessCallback,
			static_cast<void*>(this));
	}
}

void queue::process() {
	if (!_list.process()) {
		return;
	}
	_queued.store(false);

	if (!_list.empty()) {
		wake_async();
	}
}

queue::~queue() {
	if (_list.push_sentinel()) {
		wake_async();
	}
}

void queue::ProcessCallback(void *that) {
	static_cast<queue*>(that)->process();
}

} // namespace crl

#endif // CRL_USE_DISPATCH
