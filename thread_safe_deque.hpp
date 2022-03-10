// revision 1.7 by luj 
// for cross platform

#pragma once
#ifndef __SAFE_DEQUE_HPP__
#define __SAFE_DEQUE_HPP__

#include <deque>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <memory>

template <typename T, typename Alloc = std::allocator<T>>
class safe_deque : public std::deque<T, Alloc>, public std::mutex, public std::condition_variable
{
public:

#if __cplusplus > 201402L && (defined(__GNUC__) ? __GNUC__ > 6 : true)
	//if you want this code in MSVC, add this C++ Commandline Option [ /Zc:__cplusplus /std:c++17 ]
	~safe_deque() {
		if constexpr (std::is_pointer_v<T>) {
			T del;
			while (pull_front(del))
				delete del;
		}
	}
#else 
	~safe_deque() {
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

		return std::shared_ptr<T_nptr>(tmp, [&](T del){ push_back(del); });
	}

	inline bool empty() {
		std::lock_guard<std::mutex> lock(*this);
		return this->std::deque<T, Alloc>::empty();
	}

	inline size_t size() {
		std::lock_guard<std::mutex> lock(*this);
		return this->std::deque<T, Alloc>::size();
	}

	inline void push_back_notify(const T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::push_back(_Val);
		std::condition_variable::notify_one();
	}

	inline void push_back(const T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::push_back(_Val);
	}

	inline void push_back_notify(T&& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::push_back(_Val);
		std::condition_variable::notify_one();
	}

	inline void push_back(T&& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::push_back(_Val);
	}

	inline void push_front(const T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::push_front(_Val);
	}

	inline void push_front(T&& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::push_front(_Val);
	}

	inline void push_front_notify(const T& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::push_front(_Val);
		std::condition_variable::notify_one();
	}

	inline void push_front_notify(T&& _Val) {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::push_front(_Val);
		std::condition_variable::notify_one();
	}

	// non blocking function
	// return true : pull front success
	// return false : fail to pull
	inline bool pull_front(T& item) {
		std::lock_guard<std::mutex> lock(*this);
		if (!this->std::deque<T, Alloc>::empty()) {
			item = std::move(this->std::deque<T, Alloc>::front());
			this->std::deque<T, Alloc>::pop_front();
			return true;
		}
		return false;
	}

	inline bool try_get_front(T& item) {
		std::lock_guard<std::mutex> lock(*this);
		if (!this->std::deque<T, Alloc>::empty()) {
			item = this->std::deque<T, Alloc>::front();
			return true;
		}
		return false;
	}

	inline bool try_get_back(T& item) {
		std::lock_guard<std::mutex> lock(*this);
		if (!this->std::deque<T, Alloc>::empty()) {
			item = this->std::deque<T, Alloc>::back();
			return true;
		}
		return false;
	}

	inline T& back() {
		std::lock_guard<std::mutex> lock(*this);
		return this->std::deque<T, Alloc>::back();
	}

	inline T& front() {
		std::lock_guard<std::mutex> lock(*this);
		return this->std::deque<T, Alloc>::front();
	}

	inline void pop_front() {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::pop_front();
	}

	inline void pop_back() {
		std::lock_guard<std::mutex> lock(*this);
		this->std::deque<T, Alloc>::pop_back();
	}

	// blocking funtion
	// return true : pull front success
	// return false : fail to pull, and other than push_back destroys wait
	inline bool pull_front_wait(T& item) {
		std::unique_lock<std::mutex> lock(*this);

		bool empty = this->std::deque<T, Alloc>::empty();
		while (empty) {
			std::condition_variable::wait(lock);
			if (empty = this->std::deque<T, Alloc>::empty())
				return false;
		}
		item = std::move(this->std::deque<T, Alloc>::front());
		this->std::deque<T, Alloc>::pop_front();
		return true;
	}

	template<class _Rep, class _Period>
	bool pull_front_wait(T& item, const std::chrono::duration<_Rep, _Period>& _Rel_time) {
		std::unique_lock<std::mutex> lock(*this);
		if (std::deque<T, Alloc>::empty()) {
			wait_for(lock, _Rel_time);
			if (std::deque<T, Alloc>::empty())
				return false;
		}
		item = std::move(this->std::deque<T, Alloc>::front());
		this->std::deque<T, Alloc>::pop_front();
		return true;
	}

	inline void wait_break() {
		std::lock_guard<std::mutex> lock(*this);
		std::condition_variable::notify_one();
	}
};


#endif // !__SAFE_DEQUE_HPP__