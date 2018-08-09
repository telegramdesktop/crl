/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <crl/common/crl_common_config.h>
#include <crl/crl_queue.h>
#include <crl/crl_on_main.h>
#include <memory>

#ifdef CRL_ENABLE_RPL_INTEGRATION
#include <rpl/producer.h>
#endif // CRL_ENABLE_RPL_INTEGRATION

namespace crl {
namespace details {

template <typename Type>
class object_on_queue_data final
	: public std::enable_shared_from_this<object_on_queue_data<Type>> {
public:
	using Object = Type;

	template <typename ...Args>
	void construct(Args &&...args);

	template <typename Method>
	void with(Method &&method);

	template <typename Method>
	void with(Method &&method) const;

	template <typename Value>
	void destroy(Value &value) const;

	~object_on_queue_data();

private:
	template <typename Callable>
	void async(Callable &&callable) const;

	Type &value();
	const Type &value() const;

	std::aligned_storage_t<sizeof(Type), alignof(Type)> _storage;
	mutable crl::queue _queue;

};

} // namespace details

template <typename Type>
class weak_on_queue final {
	using data = details::object_on_queue_data<std::remove_const_t<Type>>;
	using my_data = std::conditional_t<
		std::is_const_v<Type>,
		const data,
		data>;

public:
	weak_on_queue() = default;
	weak_on_queue(const std::shared_ptr<data> &strong);
	weak_on_queue(const weak_on_queue &other) = default;
	weak_on_queue(weak_on_queue &&other) = default;
	weak_on_queue &operator=(const weak_on_queue &other) = default;
	weak_on_queue &operator=(weak_on_queue &&other) = default;

	template <typename Method>
	void with(Method &&method) const;

	template <typename Value>
	void destroy(Value &value) const;

	// Returns a lambda that runs arbitrary callable on the objects queue.
	// const auto r = runner(); r([] { make_some_work_on_queue(); });
	auto runner() const {
		return [weak = *this](auto &&method) {
			weak.with([
				method = std::forward<decltype(method)>(method)
			](Type&) mutable {
				std::move(method)();
			});
		};
	}

#ifdef CRL_ENABLE_RPL_INTEGRATION
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
#endif // CRL_ENABLE_RPL_INTEGRATION

private:
	std::weak_ptr<my_data> _weak;

};

template <typename Type>
class object_on_queue final {
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

#ifdef CRL_ENABLE_RPL_INTEGRATION
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
#endif // CRL_ENABLE_RPL_INTEGRATION

	weak_on_queue<Type> weak();
	weak_on_queue<const Type> weak() const;

	~object_on_queue();

private:
	using Data = details::object_on_queue_data<Type>;
	std::shared_ptr<Data> _data;

};

namespace details {

template <typename Type>
template <typename Callable>
void object_on_queue_data<Type>::async(Callable &&callable) const {
	_queue.async([
		that = this->shared_from_this(),
		what = std::forward<Callable>(callable)
	]() mutable {
		std::move(what)();
	});
}

template <typename Type>
Type &object_on_queue_data<Type>::value() {
	return *reinterpret_cast<Type*>(&_storage);
}

template <typename Type>
const Type &object_on_queue_data<Type>::value() const {
	return *reinterpret_cast<const Type*>(&_storage);
}

template <typename Type>
template <typename ...Args>
void object_on_queue_data<Type>::construct(Args &&...args) {
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
void object_on_queue_data<Type>::with(Method &&method) {
	async([=, method = std::forward<Method>(method)]() mutable {
		std::move(method)(value());
	});
}

template <typename Type>
template <typename Method>
void object_on_queue_data<Type>::with(Method &&method) const {
	async([=, method = std::forward<Method>(method)]() mutable {
		std::move(method)(value());
	});
}

template <typename Type>
template <typename Value>
void object_on_queue_data<Type>::destroy(Value &value) const {
	_queue.async([moved = std::move(value)]{});
}

template <typename Type>
object_on_queue_data<Type>::~object_on_queue_data() {
	value().~Type();
}

} // namespace details

template <typename Type>
weak_on_queue<Type>::weak_on_queue(const std::shared_ptr<data> &strong)
: _weak(strong) {
}

template <typename Type>
template <typename Method>
void weak_on_queue<Type>::with(Method &&method) const {
	if (auto strong = _weak.lock()) {
		strong->with(std::move(method));
		strong->destroy(strong);
	}
}

template <typename Type>
template <typename Value>
void weak_on_queue<Type>::destroy(Value &value) const {
	if (auto strong = _weak.lock()) {
		strong->destroy(value);
		strong->destroy(strong);
	}
}

#ifdef CRL_ENABLE_RPL_INTEGRATION
template <typename Type>
template <typename Method, typename Callback, typename Result>
Result weak_on_queue<Type>::producer(
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
template <typename Method, typename Result>
Result weak_on_queue<Type>::producer_on_main(Method &&method) const {
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
#endif // CRL_ENABLE_RPL_INTEGRATION

template <typename Type>
template <typename ...Args>
object_on_queue<Type>::object_on_queue(Args &&...args)
: _data(std::make_shared<Data>()) {
	constexpr auto plain_construct = std::is_constructible_v<
		Type,
		Args...>;
	constexpr auto with_weak_construct = std::is_constructible_v<
		Type,
		weak_on_queue<Type>,
		Args...>;
	if constexpr (plain_construct) {
		_data->construct(std::forward<Args>(args)...);
	} else if constexpr (with_weak_construct) {
		_data->construct(weak(), std::forward<Args>(args)...);
	} else {
		static_assert(false_t(args...), "Could not find a constructor.");
	}
}

template <typename Type>
template <typename Method>
void object_on_queue<Type>::with(Method &&method) {
	_data->with(std::forward<Method>(method));
}

template <typename Type>
template <typename Method>
void object_on_queue<Type>::with(Method &&method) const {
	const auto data = static_cast<const Data*>(_data.get());
	data->with(std::forward<Method>(method));
}

template <typename Type>
template <typename Value>
void object_on_queue<Type>::destroy(Value &value) const {
	_data->destroy(value);
}

#ifdef CRL_ENABLE_RPL_INTEGRATION
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
#endif // CRL_ENABLE_RPL_INTEGRATION

template <typename Type>
auto object_on_queue<Type>::weak() -> weak_on_queue<Type> {
	return { _data };
}

template <typename Type>
auto object_on_queue<Type>::weak() const -> weak_on_queue<const Type> {
	return { _data };
}

template <typename Type>
object_on_queue<Type>::~object_on_queue() {
	_data->destroy(_data);
}

} // namespace
