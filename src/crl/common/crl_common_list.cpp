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
#include <crl/common/crl_common_list.h>

#if defined CRL_USE_COMMON_LIST

#include <crl/winapi/crl_winapi_dll.h>
#include <windows.h>

namespace crl::details {

auto list::ReverseList(BasicEntry *entry, BasicEntry *next) -> BasicEntry* {
	entry->next = nullptr;
	do {
		auto third = next->next;
		next->next = entry;
		entry = next;
		next = third;
	} while (next);
	return entry;
}

list::list() = default;

bool list::push_entry(BasicEntry *entry) {
	auto head = (BasicEntry*)nullptr;
	while (true) {
		if (_head.compare_exchange_weak(head, entry)) {
			return (head == nullptr);
		}
		entry->next = head;
	}
}

bool list::push_sentinel() {
	return push_entry(AllocateSentinel());
}

bool list::empty() const {
	return (_head == nullptr);
}

bool list::process() {
	if (auto entry = _head.exchange(nullptr)) {
		if (auto next = entry->next) {
			entry = ReverseList(entry, next);
		}
		do {
			auto basic = entry;
			entry = entry->next;

			if (!basic->process) {
				// Sentinel.
				delete basic;
				_semaphore.release();
				return false;
			}
			basic->process(basic);
		} while (entry);
	}
	return true;
}

list::~list() {
	_semaphore.acquire();
}

auto list::AllocateSentinel() -> BasicEntry* {
	return new BasicEntry(nullptr);
}

} // namespace crl::details

#endif // CRL_USE_COMMON_LIST
