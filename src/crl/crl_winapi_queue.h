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

#include <crl/crl_utils.h>
#include <crl/crl_winapi_semaphore.h>
#include <atomic>

namespace crl {

class queue {
public:
	using ProcessCaller = void(*)(void (*callable)(void*), void *argument);

	queue();

	template <typename Callable>
	void async(Callable &&callable) {
		schedule(AllocateEntry(std::forward<Callable>(callable)));
	}

	template <typename Callable>
	void sync(Callable &&callable) {
		semaphore waiter;
		async([&] {
			callable();
			waiter.release();
		});
		waiter.acquire();
	}

	~queue();

private:
#if defined CRL_WINAPI_X64
	static constexpr auto kLockFreeAlignment = 16;
#elif defined CRL_WINAPI_X86 // CRL_WINAPI_X64
	static constexpr auto kLockFreeAlignment = 8;
#else // CRL_WINAPI_X86
#error "Configuration is not supported."
#endif // !CRL_WINAPI_X86 && !CRL_WINAPI_X64

	// Hide WinAPI SLIST_HEADER
	struct alignas(kLockFreeAlignment) lock_free_list {
		void *Next__; // Hide WinAPI SLIST_ENTRY
		unsigned short Depth__; // Hide WinAPI WORD
		unsigned short CpuId__; // Hide WinAPI WORD
	};

	struct alignas(kLockFreeAlignment) BasicEntry;
	using ProcessEntryMethod = void(*)(BasicEntry *entry);

	struct alignas(kLockFreeAlignment) BasicEntry {
		void *plain; // Hide WinAPI SLIST_ENTRY
		ProcessEntryMethod process;
	};

	static_assert(std::is_pod_v<BasicEntry>);
	static_assert(std::is_standard_layout_v<BasicEntry>);
	static_assert(offsetof(BasicEntry, plain) == 0);

	template <typename Function>
	struct Entry : BasicEntry {
		Entry(Function &&function) : function(std::move(function)) {
		}
		Entry(const Function &function) : function(function) {
		}
		Function function;

		static void Process(BasicEntry *entry) {
			auto full = static_cast<Entry*>(entry);
			auto guard = details::finally([=] { delete full; });
			full->function();
		}

	};

	template <typename Callable>
	static Entry<std::decay_t<Callable>> *AllocateEntry(
			Callable &&callable) {
		using Function = std::decay_t<Callable>;
		using Type = Entry<Function>;

		auto result = new Type(std::forward<Callable>(callable));
		result->process = &Type::Process;
		return result;
	}

	static BasicEntry *AllocateSentinel();
	static void ProcessCallback(void *that);

	queue(ProcessCaller caller);

	void schedule(BasicEntry *entry);
	void wake_async();
	bool empty() const;
	void process();

	ProcessCaller _caller;
	const std::unique_ptr<lock_free_list> _list;
	semaphore _semaphore;
	std::atomic<bool> _queued = false;

};

} // namespace crl
