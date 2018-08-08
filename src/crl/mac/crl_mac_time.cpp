/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include <crl/crl_time.h>

#ifdef CRL_USE_MAC_TIME

#include <mach/mach_time.h>

namespace crl::details {
namespace {

double Frequency/* = 0.*/;

} // namespace

void init() {
	mach_timebase_info_data_t tb = { 0, 0 };
	mach_timebase_info(&tb);
	Frequency = (float64(tb.numer) / tb.denom) / 1000000.;
}

inner_time_type current_value() {
	return mach_absolute_time();
}

time_type convert(inner_time_type value) {
	return time_type(value * Frequency);
}

} // namespace crl::details

#endif // CRL_USE_MAC_TIME
