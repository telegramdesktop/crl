/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include <crl/common/crl_common_list.h>

#if defined CRL_USE_COMMON_LIST

namespace crl::details {

list::list(semaphore *sentinel_semaphore)
: _sentinel_semaphore(sentinel_semaphore) {
}

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
				_sentinel_semaphore->release();
				return false;
			}
			basic->process(basic);
		} while (entry);
	}
	return true;
}

list::~list() {
	if (_sentinel_semaphore) {
		_sentinel_semaphore->acquire();
	}
}

auto list::AllocateSentinel() -> BasicEntry* {
	return new BasicEntry(nullptr);
}

} // namespace crl::details

#endif // CRL_USE_COMMON_LIST
