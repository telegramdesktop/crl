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

#if defined CRL_USE_DISPATCH && !defined CRL_USE_COMMON_QUEUE

#include <crl/dispatch/crl_dispatch_async.h>
#include <crl/common/crl_common_utils.h>

namespace crl {

inline void init_main_queue(queue_processor processor) {
}

template <typename Callable>
inline void on_main(Callable &&callable) {
	return details::on_queue_invoke(
		details::main_queue_dispatch(),
		details::on_queue_async,
		std::forward<Callable>(callable));
}

template <typename Callable>
inline void on_main_sync(Callable &&callable) {
	return details::on_queue_sync(
		details::main_queue_dispatch(),
		details::on_queue_sync,
		std::forward<Callable>(callable));
}

} // namespace crl

#endif // CRL_USE_DISPATCH && !CRL_USE_COMMON_QUEUE
