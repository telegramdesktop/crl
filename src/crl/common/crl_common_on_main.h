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

#if defined CRL_USE_COMMON_QUEUE || !defined CRL_USE_DISPATCH

#include <crl/common/crl_common_queue.h>
#include <atomic>

namespace crl::details {

extern queue *MainQueue;
extern std::atomic<int> MainQueueCounter;

class main_queue_pointer {
public:
	main_queue_pointer() {
		grab();
	}

	void create(queue_processor processor);

	explicit operator bool() const {
		return _pointer != nullptr;
	}

	queue *operator->() const {
		return _pointer;
	}

	~main_queue_pointer() {
		ungrab();
	}

private:
	void grab();
	void ungrab();

	queue *_pointer = nullptr;

};

} // namespace crl::details

namespace crl {

void init_main_queue(queue_processor processor);

template <typename Callable>
inline void on_main(Callable &&callable) {
	if (const auto main = details::main_queue_pointer()) {
		main->async(std::forward<Callable>(callable));
	}
}

template <typename Callable>
inline void on_main_sync(Callable &&callable) {
	if (const auto main = details::main_queue_pointer()) {
		main->sync(std::forward<Callable>(callable));
	}
}

} // namespace crl

#endif // CRL_USE_COMMON_QUEUE || !CRL_USE_DISPATCH
