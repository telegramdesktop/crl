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
#include <crl/winapi/crl_winapi_list.h>

#ifdef CRL_USE_WINAPI_LIST

#include <crl/winapi/crl_winapi_dll.h>
#include <windows.h>

namespace crl::details {
namespace {

PSLIST_HEADER UnwrapList(void *wrapped) {
	return static_cast<PSLIST_HEADER>(wrapped);
}

PSLIST_ENTRY UnwrapEntry(void *wrapped) {
	return static_cast<PSLIST_ENTRY>(wrapped);
}

SLIST_ENTRY *ReverseList(SLIST_ENTRY *entry, SLIST_ENTRY *next) {
	entry->Next = nullptr;
	do {
		auto third = next->Next;
		next->Next = entry;
		entry = next;
		next = third;
	} while (next);
	return entry;
}

PSLIST_ENTRY (NTAPI *RtlFirstEntrySList)(const SLIST_HEADER *ListHead) = nullptr;

} // namespace

list::list() : _impl(std::make_unique<lock_free_list>()) {
	static auto initialize = [] {
		const auto library = details::dll(
			L"ntdll.dll",
			details::dll::own_policy::load_and_leak);
		library.load(RtlFirstEntrySList, "RtlFirstEntrySList");
		return true;
	}(); // TODO crl::once?..

	static_assert(alignof(lock_free_list) == MEMORY_ALLOCATION_ALIGNMENT);
	static_assert(alignof(lock_free_list) >= alignof(SLIST_HEADER));
	static_assert(sizeof(lock_free_list) == sizeof(SLIST_HEADER));
	InitializeSListHead(UnwrapList(_impl.get()));
}

bool list::push_entry(BasicEntry *entry) {
	return (InterlockedPushEntrySList(
		UnwrapList(_impl.get()),
		UnwrapEntry(&entry->plain)) == nullptr);
}

bool list::push_sentinel() {
	return push_entry(AllocateSentinel());
}

bool list::empty() const {
	return RtlFirstEntrySList(UnwrapList(_impl.get())) == nullptr;
}

bool list::process() {
	if (auto entry = InterlockedFlushSList(UnwrapList(_impl.get()))) {
		if (auto next = entry->Next) {
			entry = ReverseList(entry, next);
		}
		do {
			auto basic = reinterpret_cast<BasicEntry*>(entry);
			entry = entry->Next;

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
	auto result = new BasicEntry();
	result->process = nullptr;
	return result;
}

} // namespace crl::details

#endif // CRL_USE_WINAPI_LIST
