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

	[[nodiscard]] T* Get();
	[[nodiscard]] const T* Get() const;
	void Reset();

	inline const size_t GetHash() const
	{
		return std::hash<void*>()(m_weakPtr.lock().get());
	}

	template<typename U>
	[[nodiscard]] Weak<U> As();

	Weak<T>& operator=(const Weak<T>& other);

	template<typename U>
	Weak<T>& operator=(const Weak<U>& other);

	bool operator==(const Weak<T>& rhs) const;

	template<typename U>
	bool operator==(const Weak<U>& rhs) const;

	inline operator bool() const { return !m_weakPtr.expired(); }
	inline operator std::shared_ptr<T>() const { return m_weakPtr.lock(); }

	[[nodiscard]] inline std::shared_ptr<T> GetSharedPtr() const { return m_weakPtr.lock(); }

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
inline Weak<U> Weak<T>::As()
{
	Weak<U> weak{};
	weak.m_weakPtr = std::reinterpret_pointer_cast<U>(m_weakPtr.lock());

	return weak;
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
inline const T* Weak<T>::Get() const
{
	return m_weakPtr.lock().get();
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

template<typename T>
inline bool Weak<T>::operator==(const Weak<T>& rhs) const
{
	return m_weakPtr.lock().get() == rhs.m_weakPtr.lock().get();
}

template<typename T>
template<typename U>
inline bool Weak<T>::operator==(const Weak<U>& rhs) const
{
	return Get() == rhs.Get();
}


template<typename T>
template<typename U>
inline Weak<T>& Weak<T>::operator=(const Weak<U>& other)
{
	m_weakPtr = std::reinterpret_pointer_cast<T>(other.GetSharedPtr());
	return *this;
}

namespace std
{
	template<typename T> struct hash;

	template<class Ty>
	struct hash<Weak<Ty>>
	{
		std::size_t operator()(const Weak<Ty> ptr) const
		{
			return std::hash<void*>()(ptr.Get());
		}
	};
}
