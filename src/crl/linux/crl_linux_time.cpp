/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include <crl/crl_time.h>

#ifdef CRL_USE_LINUX_TIME

#include <time.h>

namespace crl::details {

void init() {
}

inner_time_type current_value() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	const auto seconds = inner_time_type(ts.tv_sec);
	const auto milliseconds = inner_time_type(ts.tv_nsec) / 1000000;
	return seconds * 1000 + milliseconds;
}

time_type convert(inner_time_type value) {
	return time_type(value);
}

} // namespace crl::details

#endif // CRL_USE_LINUX_TIME
