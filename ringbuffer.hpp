// revision 1.2 by luj 
// for cross platform

#pragma once
#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <array>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <iterator>
#include <chrono>
#include <memory>
#include "assert.h"

template <class T, size_t _Size, bool use_sum = false>
class ringbuffer :
	protected std::array<T, _Size>
{
private:
	static constexpr bool sum_en = !std::is_pointer<T>::value && use_sum;
	typedef std::array<T, _Size> _Arr;
public:

#if __cplusplus > 201402L && (defined(__GNUC__) ? __GNUC__ > 6 : true)
	//if you want this code in MSVC, add this C++ Commandline Option [ /Zc:__cplusplus /std:c++17 ]
	~ringbuffer() {
		if constexpr (std::is_pointer_v<T>) {
			T del;
			while (pull_front(del))
				delete del;
		}
	}
#else 
	~ringbuffer() {
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
		return std::shared_ptr<T_nptr>(tmp, [&](T del) { push_back_force(del); });
	}

	// return false : buffer is full
	bool push_back(T& _Val) {
		if (!full()) {
			incr_back(_Val);
			return true;
		}
		return false;
	}

	// return false : buffer is full
	bool push_back(const T& _Val) {
		if (!full()) {
			incr_back(_Val);
			return true;
		}
		return false;
	}

	void push_back_force(T& _Val) {
		_if_full_delete_once();
		incr_back(_Val);
	}

	void push_back_force(const T& _Val) {
		_if_full_delete_once();
		incr_back(_Val);
	}

	// return false : buffer is empty
	bool pull_front(T& item) {
		if (_empty())
			return false;
		item = incr_front();
		return true;
	}

	auto empty() {
		return _size == 0;
	}

	auto full() {
		return _size == capacity();
	}

	auto size() noexcept {
		return _size;
	}

	void fill(const T& _Val) {
		_Arr::fill(_Val);
		if constexpr (sum_en)
			_sum = _Val * _Size;
	}

	void clear() {
		back_it = _Arr::begin();
		front_it = _Arr::begin();
		_size = 0;
		if constexpr (sum_en)
			_sum = 0;
	}

	constexpr auto capacity() const noexcept {
		return _Size;
	}

	auto begin() {
		return front_it;
	}

	auto end() {
		return back_it;
	}

	auto& front() {
		return *front_it;
	}

	auto& back() {
		return *back_it;
	}

	template <class _Ty = T>
	std::enable_if_t<sum_en, _Ty> sum() {
		return _sum;
	}

private:

	template <class _Ty = T, typename std::enable_if<std::is_pointer<_Ty>::value, _Ty>::type* = nullptr>
	inline void _if_full_delete_once() {
		if (full()) {
			T dummy = incr_front();
			delete dummy;
		}
	}

	template <class _Ty = T, typename std::enable_if< !std::is_pointer<_Ty>::value, _Ty>::type* = nullptr>
	inline void _if_full_delete_once() {
		if (full()) {
			T dummy = incr_front();
			if constexpr (sum_en)
				_sum -= dummy;
		}
	}

	inline bool _empty() {
		return _size == 0;
	}

	void incr_back(const T& _Val) {
		assert(!full() && "ringbuffer overrun");
		++_size;
		*back_it = std::move(_Val);
		if (++back_it == _Arr::end())
			back_it = _Arr::begin();
		if constexpr (sum_en)
			_sum += _Val;
	};

	void incr_back(T&& _Val) {
		assert(!full() && "ringbuffer overrun");
		++_size;
		*back_it = std::move(_Val);
		if (++back_it == _Arr::end())
			back_it = _Arr::begin();
		if constexpr (sum_en)
			_sum += _Val;
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
	T _sum;
};

#endif // !_RINGBUFFER_H_


