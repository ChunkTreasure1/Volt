#pragma once

#include <memory>

template<typename T>
class Weak
{
public:
	Weak() = default;
	Weak(std::shared_ptr<T> sharedPtr);
	Weak(std::weak_ptr<T> weakPtr);

	template<typename U>
	Weak(std::weak_ptr<U> weakPtr);

	template<typename U>
	Weak(std::shared_ptr<U> sharedPtr);

	T* operator->();
	T& operator*();

	T* operator->() const;
	T& operator*() const;

	T* Get();
	void Reset();
	
	Weak<T>& operator=(const Weak<T>& other);

	operator bool() const { return !m_weakPtr.expired(); }
	operator std::shared_ptr<T>() const { return m_weakPtr.lock(); }

private:
	std::weak_ptr<T> m_weakPtr;
};

template<typename T>
inline Weak<T>::Weak(std::shared_ptr<T> sharedPtr)
	: m_weakPtr(sharedPtr)
{
}

template<typename T>
inline Weak<T>::Weak(std::weak_ptr<T> weakPtr)
	: m_weakPtr(weakPtr)
{
}

template<typename T>
template<typename U>
inline Weak<T>::Weak(std::shared_ptr<U> sharedPtr)
	: m_weakPtr(std::reinterpret_pointer_cast<T>(sharedPtr))
{
}

template<typename T>
template<typename U>
inline Weak<T>::Weak(std::weak_ptr<U> weakPtr)
	: m_weakPtr(std::reinterpret_pointer_cast<T>(weakPtr.lock()))
{
}

template<typename T>
inline T* Weak<T>::operator->()
{
	return m_weakPtr.lock().get();
}

template<typename T>
inline T& Weak<T>::operator*()
{
	return *m_weakPtr.lock();
}

template<typename T>
inline T* Weak<T>::operator->() const
{
	return m_weakPtr.lock().get();
}

template<typename T>
inline T& Weak<T>::operator*() const
{
	return *m_weakPtr.lock();
}

template<typename T>
inline T* Weak<T>::Get()
{
	return m_weakPtr.lock().get();
}

template<typename T>
inline void Weak<T>::Reset()
{
	m_weakPtr.reset();
}

template<typename T>
inline Weak<T>& Weak<T>::operator=(const Weak<T>& other)
{
	m_weakPtr = other.m_weakPtr;
	return *this;
}
