#pragma once

#include "RefPtr.h"

template<typename T>
class WeakPtr
{
public:
	WeakPtr() noexcept = default;
	WeakPtr(T* ptr) noexcept
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
			m_weakCounter = nullptr;
		}

		m_object = ptr;
		if (m_object)
		{
			m_weakCounter = m_object->GetWeakCounter();
			m_weakCounter->Increase();
		}
	}

	WeakPtr(RefPtr<T> refPtr) noexcept
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
			m_weakCounter = nullptr;
		}

		m_object = refPtr.GetRaw();
		if (m_object)
		{
			m_weakCounter = m_object->GetWeakCounter();
			m_weakCounter->Increase();
		}
	}

	template<typename U>
	WeakPtr(const RefPtr<U>& refPtr) noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
			m_weakCounter = nullptr;
		}

		m_object = reinterpret_cast<T*>(refPtr.GetRaw());
		if (m_object)
		{
			m_weakCounter = m_object->GetWeakCounter();
			m_weakCounter->Increase();
		}
	}

	WeakPtr(const WeakPtr<T>& weakPtr) noexcept
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}

		m_weakCounter = weakPtr.m_weakCounter;
		m_object = weakPtr.m_object;

		if (m_weakCounter)
		{
			m_weakCounter->Increase();
		}
	}

	WeakPtr(WeakPtr<T>&& other) noexcept
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}

		m_object = other.m_object;
		m_weakCounter = other.m_weakCounter;
	
		if (m_weakCounter)
		{
			m_weakCounter->Increase();
		}
	}

	template<typename U>
	WeakPtr(WeakPtr<U>&& other) noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}

		m_object = reinterpret_cast<T*>(other.m_object);
		m_weakCounter = other.m_weakCounter;

		if (m_weakCounter)
		{
			m_weakCounter->Increase();
		}
	}

	template<typename U>
	WeakPtr(const WeakPtr<U>& other) noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}

		m_object = reinterpret_cast<T*>(other.m_object);
		m_weakCounter = other.m_weakCounter;

		if (m_weakCounter)
		{
			m_weakCounter->Increase();
		}
	}

	~WeakPtr() noexcept
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}
		m_object = nullptr;
		m_weakCounter = nullptr;
	}

	VT_NODISCARD VT_INLINE T* GetRaw() noexcept
	{
		return m_object;
	}

	VT_NODISCARD VT_INLINE T* GetRaw() const noexcept
	{
		return m_object;
	}

	VT_NODISCARD VT_INLINE const size_t GetHash() const
	{
		return std::hash<void*>()(m_object);
	}

	VT_NODISCARD VT_INLINE void Reset() noexcept
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}

		m_object = nullptr;
		m_weakCounter = nullptr;
	}

	VT_NODISCARD VT_INLINE bool IsValid() const
	{
		return m_weakCounter && m_weakCounter->IsValid();
	}

	template<typename U>
	VT_NODISCARD VT_INLINE WeakPtr<U> As() const noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		return WeakPtr<U>(const_cast<WeakPtr<T>&>(*this));
	}

	VT_NODISCARD VT_INLINE T* operator->() noexcept
	{
		return m_object;
	}

	VT_NODISCARD VT_INLINE T& operator*() noexcept
	{
		return *m_object;
	}

	VT_NODISCARD VT_INLINE T* operator->() const noexcept
	{
		return m_object;
	}

	VT_NODISCARD VT_INLINE T& operator*() const noexcept
	{
		return *m_object;
	}

	VT_INLINE WeakPtr<T>& operator=(const WeakPtr<T>& other) noexcept
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}

		m_object = other.m_object;
		m_weakCounter = other.m_weakCounter;

		if (m_weakCounter)
		{
			m_weakCounter->Increase();
		}
	
		return *this;
	}

	VT_INLINE WeakPtr<T>& operator=(WeakPtr<T>&& other) noexcept
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}

		m_object = other.m_object;
		m_weakCounter = other.m_weakCounter;

		if (m_weakCounter)
		{
			m_weakCounter->Increase();
		}

		return *this;
	}

	template<typename U>
	VT_INLINE WeakPtr<T>& operator=(WeakPtr<U>&& other) noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}

		m_object = reinterpret_cast<T*>(other.m_object);
		m_weakCounter = other.m_weakCounter;

		if (m_weakCounter)
		{
			m_weakCounter->Increase();
		}

		return *this;
	}

	template<typename U>
	VT_INLINE WeakPtr<T>& operator=(const WeakPtr<U>& other) noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		if (m_weakCounter)
		{
			m_weakCounter->Decrease();
		}

		m_object = reinterpret_cast<T*>(other.m_object);
		m_weakCounter = other.m_weakCounter;

		if (m_weakCounter)
		{
			m_weakCounter->Increase();
		}

		return *this;
	}

	VT_NODISCARD VT_INLINE bool operator==(const WeakPtr<T>& rhs) const noexcept
	{
		return m_object == rhs.m_object;
	}

	VT_NODISCARD VT_INLINE operator bool() const { return m_object != nullptr; }

private:
	template<typename U>
	friend class WeakPtr;

	T* m_object = nullptr;
	mutable WeakCounter* m_weakCounter = nullptr;
};

namespace std
{
	template<typename T> struct hash;

	template<class Ty>
	struct hash<WeakPtr<Ty>>
	{
		std::size_t operator()(const WeakPtr<Ty> ptr) const
		{
			return ptr.GetHash();
		}
	};
}
