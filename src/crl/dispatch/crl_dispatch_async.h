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

#ifdef CRL_USE_DISPATCH

#include <crl/common/crl_common_utils.h>
#include <type_traits>

namespace crl::details {

void *background_queue_dispatch();
void *main_queue_dispatch();

void on_queue_async(void *queue, void (*callable)(void*), void *argument);
void on_queue_sync(void *queue, void (*callable)(void*), void *argument);

template <
	typename Invoker,
	typename Callable,
	typename Return = decltype(std::declval<Callable>()())>
inline void on_queue_invoke(
		void *queue,
		Invoker invoker,
		Callable &&callable) {
	using Function = std::decay_t<Callable>;

	if constexpr (details::is_plain_function_v<Function, Return>) {
		using Plain = Return(*)();
		const auto copy = static_cast<Plain>(callable);
		invoker(queue, [](void *passed) {
			const auto callable = static_cast<Plain>(passed);
			(*callable)();
		}, static_cast<void*>(copy));
	} else {
		const auto copy = new Function(std::forward<Callable>(callable));
		invoker(queue, [](void *passed) {
			const auto callable = static_cast<Function*>(passed);
			const auto guard = details::finally([=] { delete callable; });
			(*callable)();
		}, static_cast<void*>(copy));
	}
}

void async_plain(void (*callable)(void*), void *argument) {
	return on_queue_async(background_queue_dispatch(), callable, argument);
}

} // namespace crl::details

namespace crl {

template <typename Callable>
inline void async(Callable &&callable) {
	return details::on_queue_invoke(
		details::background_queue_dispatch(),
		details::on_queue_async,
		std::forward<Callable>(callable));
}

template <typename Callable>
inline void sync(Callable &&callable) {
	return details::on_queue_sync(
		details::background_queue_dispatch(),
		details::on_queue_sync,
		std::forward<Callable>(callable));
}

} // namespace crl

#endif // CRL_USE_DISPATCH
