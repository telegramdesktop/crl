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

#include <windows.h>
#include <exception>

namespace crl::details {

class dll {
public:
	enum class own_policy {
		owner,
		load_and_leak,
		use_existing,
	};
	dll(LPCWSTR library, own_policy policy)
	: _handle((policy == own_policy::use_existing)
		? GetModuleHandle(library)
		: LoadLibrary(library))
	, _policy(policy) {
	}

	template <typename Function>
	bool try_load(Function &function, const char *name) const {
		if (!_handle) {
			return false;
		}
		function = reinterpret_cast<Function>(GetProcAddress(_handle, name));
		return (function != nullptr);
	}

	template <typename Function>
	void load(Function &function, const char *name) const {
		if (!try_load(function, name)) {
			Failed();
		}
	}

	~dll() {
		if (_handle && _policy == own_policy::owner) {
			FreeLibrary(_handle);
		}
	}

private:
	[[noreturn]] static void Failed() {
		throw std::exception("Could not load method from dll.");
	}

	HMODULE _handle = nullptr;
	own_policy _policy = own_policy::use_existing;

};

} // namespace crl::details
