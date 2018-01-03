/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
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
