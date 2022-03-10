#pragma once
#ifndef __TEMPLATESINGLETON_H__
#define __TEMPLATESINGLETON_H__

#include <stdlib.h>

template<class T>
class Singleton
{
protected:
	Singleton()
	{

	}
	~Singleton()
	{
		
	}

public:
	static T * get()
	{
		if (instance == nullptr) {
			instance = new T;
			atexit(destroy);
		}

		return instance;
	};

	static void destroy()
	{
		if (instance != nullptr)
		{
			delete instance;
			instance = nullptr;
		}
	};

	Singleton(const Singleton&) = delete;
	Singleton& operator= (const Singleton) = delete;

private:
	static T* instance;
};

template<class T> T* Singleton<T>::instance = nullptr;

#endif // !__TEMPLATESINGLETON_H__