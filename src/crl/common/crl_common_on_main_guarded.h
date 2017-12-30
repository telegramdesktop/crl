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

#include <memory>
#include <type_traits>

namespace crl::details {

template <typename T>
constexpr std::size_t dependent_zero = 0;

} // namespace crl::details

namespace crl {

template <typename T, typename Enable = void>
struct guard_traits;

template <
	typename Guard,
	typename Callable,
	typename GuardTraits = guard_traits<std::decay_t<Guard>>,
	typename = std::enable_if_t<
		sizeof(GuardTraits) != details::dependent_zero<GuardTraits>>>
inline void on_main(Guard &&guard, Callable &&callable) {
	on_main([
		passed_guard = GuardTraits::create(std::forward<Guard>(guard)),
		passed_callable = std::forward<Callable>(callable)
	]() mutable {
		if (GuardTraits::check(passed_guard)) {
			std::forward<Callable>(passed_callable)();
		}
	});
}

template <
	typename Guard,
	typename Callable,
	typename GuardTraits = guard_traits<std::decay_t<Guard>>,
	typename = std::enable_if_t<
		sizeof(GuardTraits) != details::dependent_zero<GuardTraits>>>
inline void on_main_sync(Guard &&guard, Callable &&callable) {
	on_main_sync([
		passed_guard = GuardTraits::create(std::forward<Guard>(guard)),
		passed_callable = std::forward<Callable>(callable)
	]() mutable {
		if (GuardTraits::check(passed_guard)) {
			std::forward<Callable>(passed_callable)();
		}
	});
}

} // namespace crl
