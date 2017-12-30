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

#if __has_include(<QtCore/QPointer>)

class QObject;

template <typename T>
class QPointer;

template <typename T>
class QWeakPointer;

template <typename T>
class QSharedPointer;

#if __has_include(<gsl/gsl>)

namespace gsl {

template <typename T>
class not_null;

} // namespace gsl

#endif // gsl

namespace crl {

template <typename T, typename Enable>
struct guard_traits;

template <typename T>
struct guard_traits<QPointer<T>, void> {
	static QPointer<T> create(const QPointer<T> &value) {
		return value;
	}
	static QPointer<T> create(QPointer<T> &&value) {
		return std::move(value);
	}
	static bool check(const QPointer<T> &guard) {
		return guard.data() != nullptr;
	}

};

template <typename T>
struct guard_traits<
	T*,
	std::enable_if_t<
		std::is_base_of_v<QObject, std::remove_cv_t<T>>>> {
	static QPointer<T> create(T *value) {
		return value;
	}
	static bool check(const QPointer<T> &guard) {
		return guard.data() != nullptr;
	}

};

#if __has_include(<gsl/gsl>)

template <typename T>
struct guard_traits<
	gsl::not_null<T*>,
	std::enable_if_t<
		std::is_base_of_v<QObject, std::remove_cv_t<T>>>> {
	static QPointer<T> create(gsl::not_null<T*> value) {
		return value.get();
	}
	static bool check(const QPointer<T> &guard) {
		return guard.data() != nullptr;
	}

};

#endif // gsl

template <typename T>
struct guard_traits<QWeakPointer<T>, void> {
	static QWeakPointer<T> create(const QWeakPointer<T> &value) {
		return value;
	}
	static QWeakPointer<T> create(QWeakPointer<T> &&value) {
		return std::move(value);
	}
	static bool check(const QWeakPointer<T> &guard) {
		return guard.toStrongRef() != nullptr;
	}

};

template <typename T>
struct guard_traits<QSharedPointer<T>, void> {
	static QWeakPointer<T> create(const QSharedPointer<T> &value) {
		return value;
	}
	static QWeakPointer<T> create(QSharedPointer<T> &&value) {
		return value;
	}
	static bool check(const QWeakPointer<T> &guard) {
		return guard.toStrongRef() != nullptr;
	}

};

} // namespace crl

#endif // Qt
