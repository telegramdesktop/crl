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

#include <crl/crl_utils.h>
#include <crl/crl_dispatch_semaphore.h>
#include <atomic>

#ifndef CRL_USE_DISPATCH
#error "This file should not be included by client-code directly."
#endif // CRL_USE_DISPATCH

namespace crl {

class queue {
public:
	queue();

	template <
		typename Callable,
		typename Return = decltype(std::declval<Callable>()())>
	void async(Callable &&callable) {
		using Function = std::decay_t<Callable>;

		if constexpr (details::is_plain_function_v<Function, Return>) {
			using Plain = Return(*)();
			const auto copy = static_cast<Plain>(callable);
			async_plain([](void *passed) {
				const auto callable = static_cast<Plain>(passed);
				(*callable)();
			}, static_cast<void*>(copy));
		} else {
			const auto copy = new Function(std::forward<Callable>(callable));
			async_plain([](void *passed) {
				const auto callable = static_cast<Function*>(passed);
				const auto guard = details::finally([=] { delete callable; });
				(*callable)();
			}, static_cast<void*>(copy));
		}
	}

	template <
		typename Callable,
		typename Return = decltype(std::declval<Callable>()())>
	void sync(Callable &&callable) {
		using Function = std::decay_t<Callable>;

		if constexpr (details::is_plain_function_v<Function, Return>) {
			using Plain = Return(*)();
			const auto copy = static_cast<Plain>(callable);
			sync_plain([](void *passed) {
				const auto callable = static_cast<Plain>(passed);
				(*callable)();
			}, static_cast<void*>(copy));
		} else {
			const auto copy = new Function(std::forward<Callable>(callable));
			sync_plain([](void *passed) {
				const auto callable = static_cast<Function*>(passed);
				const auto guard = details::finally([=] { delete callable; });
				(*callable)();
			}, static_cast<void*>(copy));
		}
	}

	~queue();

private:
	// Hide dispatch_queue_t
	struct implementation {
		using pointer = void*;
		static pointer create();
		void operator()(pointer value);
	};

	void async_plain(void (*callable)(void*), void *argument);
	void sync_plain(void (*callable)(void*), void *argument);

	std::unique_ptr<implementation::pointer, implementation> _handle;

};

} // namespace crl
