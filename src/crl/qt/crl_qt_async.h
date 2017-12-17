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

#ifdef CRL_USE_QT

#include <crl/common/crl_common_utils.h>
#include <crl/common/crl_common_sync.h>
#include <type_traits>

#include <QtCore/QThreadPool>

namespace crl::details {

template <typename Callable>
class Runnable : public QRunnable {
public:
	Runnable(Callable &&callable) : _callable(std::move(callable)) {
	}

	void run() override {
		_callable();
	}

private:
	Callable _callable;

};

template <typename Callable>
inline auto create_runnable(Callable &&callable) {
	if constexpr (std::is_reference_v<Callable>) {
		auto copy = callable;
		return create_runnable(std::move(copy));
	} else {
		return new Runnable<Callable>(std::move(callable));
	}
}

template <typename Callable>
inline void async_any(Callable &&callable) {
	QThreadPool::globalInstance()->start(
		create_runnable(std::forward<Callable>(callable)));
}

inline void async_plain(void (*callable)(void*), void *argument) {
	async_any([=] {
		callable(argument);
	});
}

} // namespace crl::details

namespace crl {

template <
	typename Callable,
	typename Return = decltype(std::declval<Callable>()())>
inline void async(Callable &&callable) {
	details::async_any(std::forward<Callable>(callable));
}

} // namespace crl

#endif // CRL_USE_QT
