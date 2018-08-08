/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <crl/common/crl_common_config.h>

#include <cstdint>

// Use after main() started.

namespace crl {

using time_type = std::int64_t;

namespace details {

using inner_time_type = std::int64_t;

void init();
inner_time_type current_value();
time_type convert(inner_time_type value);

} // namespace details

// Thread-safe.
time_type time();

// Returns true if some adjustment was made.
bool adjust_time();

} // namespace crl
