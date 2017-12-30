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

namespace crl {

template <typename T, typename Enable>
struct guard_traits;


template <typename T>
struct guard_traits<std::weak_ptr<T>, void> {
	static std::weak_ptr<T> create(const std::weak_ptr<T> &value) {
		return value;
	}
	static std::weak_ptr<T> create(std::weak_ptr<T> &&value) {
		return std::move(value);
	}
	static bool check(const std::weak_ptr<T> &guard) {
		return guard.lock() != nullptr;
	}

};

template <typename T>
struct guard_traits<std::shared_ptr<T>, void> {
	static std::weak_ptr<T> create(const std::shared_ptr<T> &value) {
		return value;
	}
	static std::weak_ptr<T> create(std::shared_ptr<T> &&value) {
		return value;
	}
	static bool check(const std::weak_ptr<T> &guard) {
		return guard.lock() != nullptr;
	}

};

} // namespace crl
