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
#include <crl/crl_winapi_queue.h>

#include <crl/crl_winapi_async.h>
#include <windows.h>

namespace crl {
namespace {

PSLIST_HEADER UnwrapList(void *wrapped) {
	return static_cast<PSLIST_HEADER>(wrapped);
}

PSLIST_ENTRY UnwrapEntry(void *wrapped) {
	return static_cast<PSLIST_ENTRY>(wrapped);
}

static SLIST_ENTRY *ReverseList(
		SLIST_ENTRY *entry,
		SLIST_ENTRY *next) {
	entry->Next = nullptr;
	do {
		auto third = next->Next;
		next->Next = entry;
		entry = next;
		next = third;
	} while (next);
	return entry;
}

} // namespace

queue::queue() : queue(&details::async_plain) {
}

queue::queue(ProcessCaller caller)
: _caller(caller)
, _list(std::make_unique<lock_free_list>()) {
	static_assert(alignof(lock_free_list) == MEMORY_ALLOCATION_ALIGNMENT);
	static_assert(sizeof(lock_free_list::Next) == sizeof(SLIST_HEADER::Next));
	static_assert(offsetof(lock_free_list, Next) == offsetof(SLIST_HEADER, Next));
	static_assert(sizeof(lock_free_list::Depth) == sizeof(SLIST_HEADER::Depth));
	static_assert(offsetof(lock_free_list, Depth) == offsetof(SLIST_HEADER, Depth));
	static_assert(sizeof(lock_free_list::CpuId) == sizeof(SLIST_HEADER::CpuId));
	static_assert(offsetof(lock_free_list, CpuId) == offsetof(SLIST_HEADER, CpuId));
	InitializeSListHead(UnwrapList(_list.get()));
}

void queue::schedule(BasicEntry *entry) {
	InterlockedPushEntrySList(
		UnwrapList(_list.get()),
		UnwrapEntry(&entry->plain));
	wake_async();
}

void queue::wake_async() {
	auto expected = false;
	if (_queued.compare_exchange_strong(expected, true)) {
		_caller(
			ProcessCallback,
			static_cast<void*>(this));
	}
}

bool queue::empty() const {
	auto pointer = &UnwrapList(_list.get())->Next.Next;
	auto plain = reinterpret_cast<void * volatile *>(pointer);
	return InterlockedCompareExchangePointerAcquire(
		plain,
		nullptr,
		nullptr) == nullptr;
}

void queue::process() {
	if (auto entry = InterlockedFlushSList(UnwrapList(_list.get()))) {
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
				return;
			}
			basic->process(basic);
		} while (entry);
	}
	_queued.store(false, std::memory_order_release);

	if (!empty()) {
		wake_async();
	}
}

queue::~queue() {
	schedule(AllocateSentinel());
	_semaphore.acquire();
}

auto queue::AllocateSentinel() -> BasicEntry* {
	auto result = new BasicEntry();
	result->process = nullptr;
	return result;
}

void queue::ProcessCallback(void *that) {
	static_cast<queue*>(that)->process();
}

} // namespace crl
