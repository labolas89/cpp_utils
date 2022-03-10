// revision 1.2 by luj 
// for cross platform

#pragma once
#ifndef __SAFE_RINGBUFFER_HPP_
#define __SAFE_RINGBUFFER_HPP_

#include <array>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <iterator>
#include <chrono>
#include <memory>
#include "assert.h"

template <class T, size_t _Size>
class safe_ringbuffer :
	protected std::array<T, _Size>,
	protected std::mutex,
	public std::condition_variable
{
private:
	typedef std::array<T, _Size> _Arr;
public:

#if __cplusplus > 201402L && (defined(__GNUC__) ? __GNUC__ > 6 : true)
	//if you want this code in MSVC, add this C++ Commandline Option [ /Zc:__cplusplus /std:c++17 ]
	~safe_ringbuffer() {
		if constexpr (std::is_pointer_v<T>) {
			T del;
			while (pull_front(del))
				delete del;
		}
	}
#else 
	~safe_ringbuffer() {
		deallocator();
	}

private:

	// only pointer template pull_front & delete
	template <class _Ty = T, typename std::enable_if<std::is_pointer<_Ty>::value, _Ty>::type* = nullptr>
	void deallocator() {
		T del;
		while (pull_front(del))
			delete del;
	}

	// not pointer template. do notting
	template <class _Ty = T, typename std::enable_if< !std::is_pointer<_Ty>::value, _Ty>::type* = nullptr>
	void deallocator() {}

public:
#endif

	template <class _Ty = T, typename std::enable_if<std::is_pointer<_Ty>::value, _Ty>::type* = nullptr>
	auto pull_front_auto_recycle() {
		using T_nptr = typename std::remove_pointer<T>::type;
		T tmp = nullptr;
		if (!pull_front(tmp)) 
			tmp = new T_nptr;
		return std::shared_ptr<T_nptr>(tmp, [&](T del){ push_back_force(del); });
	}

	// return false : buffer is full
	bool push_back(T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		if (!_full()) {
			incr_back(_Val);
			return true;
		}
		return false;
	}

	// return false : buffer is full
	bool push_back(const T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		if (!_full()) {
			incr_back(_Val);
			return true;
		}
		return false;
	}

	void push_back_force(T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		_if_full_delete_once();
		incr_back(_Val);
	}

	void push_back_force(const T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		_if_full_delete_once();
		incr_back(_Val);
	}

	// return false : buffer is full
	bool push_back_notify(T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		if (!_full()) {
			incr_back(_Val);
			notify_one();
			return true;
		}
		return false;
	}

	// return false : buffer is full
	bool push_back_notify(const T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		if (!_full()) {
			incr_back(_Val);
			notify_one();
			return true;
		}
		return false;
	}

	void push_back_force_notify(T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		_if_full_delete_once();
		incr_back(_Val);
		notify_one();
	}

	void push_back_force_notify(const T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		_if_full_delete_once();
		incr_back(_Val);
		notify_one();
	}

	// return false : buffer is empty
	bool pull_front(T& item) {
		std::lock_guard<std::mutex> lock(*this);
		if (_empty())
			return false;
		item = incr_front();
		return true;
	}

	// return false : buffer is empty
	bool pull_front_wait(T& item) {
		std::unique_lock<std::mutex> lock(*this);
		while (_empty())
		{
			wait(lock);
			if (_empty())
				return false;
		}
		item = incr_front();
		return true;
	}

	// return false : buffer is empty or wait timeout
	template<class _Rep,
		class _Period>
	bool pull_front_wait(T& item, const std::chrono::duration<_Rep, _Period>& _Rel_time) {
		std::unique_lock<std::mutex> lock(*this);
		if (_empty()) {
			wait_for(lock, _Rel_time);
			if (_empty())
				return false;
		}
		item = incr_front();
		return true;
	}

	auto empty() {
		std::lock_guard<std::mutex> lock(*this);
		return _size == 0;
	}

	auto full() {
		std::lock_guard<std::mutex> lock(*this);
		return _size == capacity();
	}

	auto size() noexcept {
		std::lock_guard<std::mutex> lock(*this);
		return _size;
	}

	void fill(const T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		_Arr::fill(_Val);
	}

	void clear() {
		std::lock_guard<std::mutex> lock(*this);
		back_it = _Arr::begin();
		front_it = _Arr::begin();
		_size = 0;
	}

	constexpr auto capacity() const noexcept {
		return _Size;
	}

private:

	inline bool _full() {
		return _size == capacity();
	}

	template <class _Ty = T, typename std::enable_if<std::is_pointer<_Ty>::value, _Ty>::type* = nullptr>
	inline void _if_full_delete_once() {
		if (_full()) {
			T dummy = incr_front();
			delete dummy;
		}
	}

	template <class _Ty = T, typename std::enable_if< !std::is_pointer<_Ty>::value, _Ty>::type* = nullptr>
	inline void _if_full_delete_once() {
		if (_full())
			T dummy = incr_front();
	}

	inline bool _empty() {
		return _size == 0;
	}

	void incr_back(const T& _Val) {
		assert(!_full() && "ringbuffer overrun");
		++_size;
		*back_it = std::move(_Val);
		if (++back_it == _Arr::end())
			back_it = _Arr::begin();
	};

	void incr_back(T&& _Val) {
		assert(!_full() && "ringbuffer overrun");
		++_size;
		*back_it = std::move(_Val);
		if (++back_it == _Arr::end())
			back_it = _Arr::begin();
	};

	T&& incr_front() {
		assert(!_empty() && "ringbuffer underrun");
		--_size;
		auto before_it = front_it++;
		if (front_it == _Arr::end())
			front_it = _Arr::begin();
		return std::move(*before_it);
	}

	typename _Arr::iterator back_it{ _Arr::begin() };
	typename _Arr::iterator front_it{ _Arr::begin() };

	size_t _size{ 0 };

};

#endif // !__SAFE_RINGBUFFER_HPP_

