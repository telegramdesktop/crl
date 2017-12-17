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
#include <utility>

namespace crl {

using queue_processor = void(*)(void (*callable)(void*), void *argument);

} // namespace crl

namespace crl::details {

using true_t = char;
struct false_t {
	char data[2];
};

template <typename Return, typename ...Args>
struct check_plain_function {
	static false_t check(...);
	static true_t check(Return(*)(Args...));
};

template <typename Callable, typename Return, typename ...Args>
constexpr bool is_plain_function_v = sizeof(
	check_plain_function<Return, Args...>::check(
		std::declval<Callable>())) == sizeof(true_t);

template <typename Callable>
class finalizer {
public:
	explicit finalizer(Callable &&callable)
	: _callable(std::move(callable)) {
	}
	finalizer(const finalizer &other) = delete;
	finalizer &operator=(const finalizer &other) = delete;
	finalizer(finalizer &&other)
	: _callable(std::move(other._callable))
	, _disabled(std::exchange(other._disabled, true)) {
	}
	finalizer &operator=(finalizer &&other) {
		_callable = std::move(other._callable);
		_disabled = std::exchange(other._disabled, true);
	}
	~finalizer() {
		if (!_disabled) {
			_callable();
		}
	}

private:
	Callable _callable;
	bool _disabled = false;

};

template <
	typename Callable,
	typename = std::enable_if_t<!std::is_reference_v<Callable>>>
finalizer<Callable> finally(Callable &&callable) {
	return finalizer<Callable>{ std::move(callable) };
}

} // namespace crl::details
