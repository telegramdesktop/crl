/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include <crl/crl_time.h>

#ifdef CRL_USE_WINAPI_TIME

#include <crl/winapi/crl_winapi_windows_h.h>

namespace crl::details {
namespace {

double Frequency/* = 0.*/;

} // namespace

void init() {
	LARGE_INTEGER value;
	QueryPerformanceFrequency(&value);
	Frequency = 1000. / double(value.QuadPart);
}

inner_time_type current_value() {
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	return value.QuadPart;
}

time convert(inner_time_type value) {
	return time(value * Frequency);
}

} // namespace crl::details

#endif // CRL_USE_WINAPI_TIME
