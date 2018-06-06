/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <crl/crl_queue.h>
#include <memory>

namespace crl {

template <typename Type>
class object_on_queue final {
	class shared;

public:
	template <typename ...Args>
	object_on_queue(Args &&...args);

	object_on_queue(const object_on_queue &other) = delete;
	object_on_queue &operator=(const object_on_queue &other) = delete;

	template <typename Method>
	void with(Method &&method);

	template <typename Method>
	void with(Method &&method) const;

	template <typename Value>
	void destroy(Value &value) const;

	template <
		typename Method,
		typename Callback,
		typename Result = decltype(
			std::declval<Method>()(std::declval<Type&>()))>
	Result producer(Method &&method, Callback &&callback) const;

	template <
		typename Method,
		typename Result = decltype(
			std::declval<Method>()(std::declval<Type&>()))>
	Result producer_on_main(Method &&method) const;

	template <typename Shared>
	class weak_wrap final {
	public:
		weak_wrap() = default;
		weak_wrap(const std::shared_ptr<shared> &strong);
		weak_wrap(const weak_wrap &other) = default;
		weak_wrap(weak_wrap &&other) = default;
		weak_wrap &operator=(const weak_wrap &other) = default;
		weak_wrap &operator=(weak_wrap &&other) = default;

		template <typename Method>
		void with(Method &&method) const;

		template <typename Value>
		void destroy(Value &value) const;

		template <
			typename Method,
			typename Callback,
			typename Result = decltype(
				std::declval<Method>()(std::declval<Type&>()))>
		Result producer(Method &&method, Callback &&callback) const;

		template <
			typename Method,
			typename Result = decltype(
				std::declval<Method>()(std::declval<Type&>()))>
		Result producer_on_main(Method &&method) const;

	private:
		std::weak_ptr<Shared> _weak;

	};
	weak_wrap<shared> weak();
	weak_wrap<const shared> weak() const;

	~object_on_queue();

private:
	class shared final : public std::enable_shared_from_this<shared> {
	public:
		template <typename ...Args>
		void construct(Args &&...args);

		template <typename Method>
		void with(Method &&method);

		template <typename Method>
		void with(Method &&method) const;

		template <typename Value>
		void destroy(Value &value) const;

		~shared();

	private:
		template <typename Callable>
		void async(Callable &&callable) const;

		Type &value();
		const Type &value() const;

		std::aligned_storage_t<sizeof(Type), alignof(Type)> _storage;
		mutable crl::queue _queue;

	};

	std::shared_ptr<shared> _content;

};

template <typename Type>
template <typename Shared>
object_on_queue<Type>::weak_wrap<Shared>::weak_wrap(
	const std::shared_ptr<shared> &strong)
: _weak(strong) {
}

template <typename Type>
template <typename Shared>
template <typename Method>
void object_on_queue<Type>::weak_wrap<Shared>::with(Method &&method) const {
	if (auto strong = _weak.lock()) {
		strong->with(std::move(method));
		strong->destroy(strong);
	}
}

template <typename Type>
template <typename Shared>
template <typename Value>
void object_on_queue<Type>::weak_wrap<Shared>::destroy(Value &value) const {
	if (auto strong = _weak.lock()) {
		strong->destroy(value);
		strong->destroy(strong);
	}
}

template <typename Type>
template <typename Shared>
template <typename Method, typename Callback, typename Result>
Result object_on_queue<Type>::weak_wrap<Shared>::producer(
		Method &&method,
		Callback &&callback) const {
	return [
		weak = *this,
		method = std::forward<Method>(method),
		callback = std::forward<Callback>(callback)
	](auto consumer) mutable {
		auto lifetime_on_queue = std::make_shared<rpl::lifetime>();
		weak.with([
			method = std::move(method),
			callback = std::move(callback),
			consumer = std::move(consumer),
			lifetime_on_queue
		](const Type &that) mutable {
			method(
				that
			) | rpl::start_with_next([
				callback = std::move(callback),
				consumer = std::move(consumer)
			](auto &&value) {
				callback(
					consumer,
					std::forward<decltype(value)>(value));
			}, *lifetime_on_queue);
		});
		return rpl::lifetime([
			lifetime_on_queue = std::move(lifetime_on_queue),
			weak = std::move(weak)
		]() mutable {
			weak.destroy(lifetime_on_queue);
		});
	};
}

template <typename Type>
template <typename Shared>
template <typename Method, typename Result>
Result object_on_queue<Type>::weak_wrap<Shared>::producer_on_main(
		Method &&method) const {
	return producer(std::forward<Method>(method), [](
			const auto &consumer,
			auto &&value) {
		crl::on_main([
			consumer,
			event = std::forward<decltype(value)>(value)
		]() mutable {
			consumer.put_next(std::move(event));
		});
	});
}

template <typename Type>
template <typename Callable>
void object_on_queue<Type>::shared::async(Callable &&callable) const {
	_queue.async([
		that = this->shared_from_this(),
		what = std::forward<Callable>(callable)
	]() mutable {
		std::move(what)();
	});
}

template <typename Type>
Type &object_on_queue<Type>::shared::value() {
	return *reinterpret_cast<Type*>(&_storage);
}

template <typename Type>
const Type &object_on_queue<Type>::shared::value() const {
	return *reinterpret_cast<const Type*>(&_storage);
}

template <typename Type>
template <typename ...Args>
void object_on_queue<Type>::shared::construct(Args &&...args) {
	async([arguments = std::make_tuple(
		&_storage,
		std::forward<Args>(args)...
	)]() mutable {
		const auto create = [](void *storage, Args &&...args) {
			new (storage) Type(std::forward<Args>(args)...);
		};
		std::apply(create, std::move(arguments));
	});
}

template <typename Type>
template <typename Method>
void object_on_queue<Type>::shared::with(Method &&method) {
	async([=, method = std::forward<Method>(method)]() mutable {
		std::move(method)(value());
	});
}

template <typename Type>
template <typename Method>
void object_on_queue<Type>::shared::with(Method &&method) const {
	async([=, method = std::forward<Method>(method)]() mutable {
		std::move(method)(value());
	});
}

template <typename Type>
template <typename Value>
void object_on_queue<Type>::shared::destroy(Value &value) const {
	_queue.async([moved = std::move(value)]{});
}

template <typename Type>
object_on_queue<Type>::shared::~shared() {
	value().~Type();
}

template <typename Type>
template <typename ...Args>
object_on_queue<Type>::object_on_queue(Args &&...args)
: _content(std::make_shared<shared>()) {
	_content->construct(std::forward<Args>(args)...);
}

template <typename Type>
template <typename Method>
void object_on_queue<Type>::with(Method &&method) {
	_content->with(std::forward<Method>(method));
}

template <typename Type>
template <typename Method>
void object_on_queue<Type>::with(Method &&method) const {
	const auto content = static_cast<const shared*>(_content.get());
	content->with(std::forward<Method>(method));
}

template <typename Type>
template <typename Value>
void object_on_queue<Type>::destroy(Value &value) const {
	_content->destroy(value);
}

template <typename Type>
template <typename Method, typename Callback, typename Result>
Result object_on_queue<Type>::producer(
		Method &&method,
		Callback &&callback) const {
	return weak().producer(
		std::forward<Method>(method),
		std::forward<Callback>(callback));
}

template <typename Type>
template <typename Method, typename Result>
Result object_on_queue<Type>::producer_on_main(Method &&method) const {
	return weak().producer_on_main(std::forward<Method>(method));
}

template <typename Type>
auto object_on_queue<Type>::weak() -> weak_wrap<shared> {
	return { _content };
}

template <typename Type>
auto object_on_queue<Type>::weak() const -> weak_wrap<const shared> {
	return { _content };
}

template <typename Type>
object_on_queue<Type>::~object_on_queue() {
	_content->destroy(_content);
}

} // namespace
