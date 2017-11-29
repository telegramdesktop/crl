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

#ifdef _MSC_VER

#if defined _WIN64
#define CRL_USE_WINAPI
#define CRL_WINAPI_X64
#elif defined _M_IX86 // _WIN64
#define CRL_USE_WINAPI
#define CRL_WINAPI_X86
#else // _M_IX86
#error "Configuration is not supported."
#endif // !_WIN64 && !_M_IX86

#define CRL_USE_WINAPI_LIST

#elif defined __APPLE__ // _MSC_VER

#define CRL_USE_DISPATCH

#elif __has_include(<QThreadPool>) // __APPLE__

#define CRL_USE_QT
#define CRL_USE_COMMON_LIST

#else // Qt
#error "Configuration is not supported."
#endif // !_MSC_VER && !__APPLE__ && !Qt
