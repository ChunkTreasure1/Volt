#pragma once

#include "CoreUtilities/Pointers/RefCounted.h"
#include "CoreUtilities/Memory/HeapAllocator.h"

#include <xhash>

template<typename T>
class RefPtr
{
public:
	RefPtr() noexcept = default;

	RefPtr(RefPtr<T>&& other) noexcept
	{
		if (m_object)
		{
			m_object->DecRef();
		}

		m_object = other.m_object;
		if (m_object)
		{
			m_object->IncRef();
		}
	}
	
	RefPtr(const RefPtr<T>& other) noexcept
	{
		if (m_object)
		{
			m_object->DecRef();
		}

		m_object = other.m_object;
		if (m_object)
		{
			m_object->IncRef();
		}
	}

	template<typename U>
	RefPtr(RefPtr<U>&& other) noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		if (m_object)
		{
			m_object->DecRef();
		}

		m_object = reinterpret_cast<T*>(other.m_object);
		if (m_object)
		{
			m_object->IncRef();
		}
	}

	template<typename U>
	RefPtr(const RefPtr<U>& other) noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		if (m_object)
		{
			m_object->DecRef();
		}

		m_object = reinterpret_cast<T*>(other.m_object);

		if (m_object)
		{
			m_object->IncRef();
		}
	}

	RefPtr(nullptr_t null) noexcept
	{ }

	~RefPtr() noexcept
	{
		if (m_object)
		{
			m_object->DecRef();
		}
		m_object = nullptr;
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
		if (m_object)
		{
			m_object->DecRef();
		}

		m_object = nullptr;
	}

	template<typename U>
	VT_NODISCARD VT_INLINE RefPtr<U> As() const noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		return RefPtr<U>(const_cast<RefPtr<T>&>(*this));
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

	VT_INLINE RefPtr<T>& operator=(const RefPtr<T>& other) noexcept
	{
		if (m_object)
		{
			m_object->DecRef();
		}

		m_object = other.m_object;

		if (m_object)
		{
			m_object->IncRef();
		}

		return *this;
	}

	template<typename U>
	VT_INLINE RefPtr<T>& operator=(const RefPtr<U>& other) noexcept requires (std::is_base_of<T, U>::value || std::is_base_of<U, T>::value)
	{
		if (m_object)
		{
			m_object->DecRef();
		}

		m_object = reinterpret_cast<T*>(other.m_object);

		if (m_object)
		{
			m_object->IncRef();
		}

		return *this;
	}

	VT_INLINE RefPtr<T>& operator=(nullptr_t null) noexcept
	{
		Reset();
		return *this;
	}

	VT_NODISCARD VT_INLINE bool operator==(const RefPtr<T>& rhs) const noexcept
	{
		return m_object == rhs.m_object;
	}

	VT_NODISCARD VT_INLINE operator bool() const { return m_object != nullptr; }

	VT_INLINE RefPtr<T>& operator=(RefPtr<T>&& other) noexcept
	{
		if (m_object)
		{
			m_object->DecRef();
		}

		m_object = other.m_object;

		if (m_object)
		{
			m_object->IncRef();
		}

		return *this;
	}

	template<typename... Args>
	VT_INLINE static RefPtr<T> Create(Args&&... args)
	{
		return RefPtr<T>(HeapAllocator::Allocate<T>(std::forward<Args>(args)...));
	}

private:
	template<typename U>
	friend class RefPtr;

	RefPtr(T* object) noexcept
		: m_object(object)
	{
		//m_object->IncRef();
	}

	T* m_object = nullptr;
};

namespace std
{
	template<typename T> struct hash;

	template<class Ty>
	struct hash<RefPtr<Ty>>
	{
		std::size_t operator()(const RefPtr<Ty> ptr) const
		{
			return ptr.GetHash();
		}
	};
}
