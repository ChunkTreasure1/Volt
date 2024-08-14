#pragma once

#include "CoreUtilities/CompilerTraits.h"
#include "CoreUtilities/GenericIterator.h"
#include "CoreUtilities/Memory/HeapAllocator.h"

#include <iterator>

namespace Internal
{
	template<typename T>
	T* AddressOf(T& value) noexcept
	{
		return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(value)));
	}

	VT_INLINE VT_NODISCARD void* Allocate(size_t size, size_t alignment)
	{
		return HeapAllocator::AllocateUninitialized(size, alignment);
	}

	VT_INLINE void Free(void* ptr)
	{
		HeapAllocator::FreeUninitialized(ptr);
	}

	VT_INLINE VT_NODISCARD void* StackAllocate(size_t size)
	{
		return VT_STACK_ALLOCATE(size);
	}

	VT_INLINE void StackFree(void* ptr)
	{
		VT_STACK_FREE(ptr);
	}
}

// Uninitialized Copy
namespace Internal
{
	template<bool isTriviallyCopyable, bool isInputIteratorReferenceAddressable, bool areIteratorsContiguous>
	struct UninitializedCopyImpl
	{
		template<typename InputIterator, typename ForwardIterator>
		static ForwardIterator Impl(InputIterator begin, InputIterator end, ForwardIterator dest)
		{
			typedef typename std::iterator_traits<ForwardIterator>::value_type valueType;
			ForwardIterator currentDest(dest);

			for (; begin != end; ++begin, ++currentDest)
			{
				::new(static_cast<void*>(AddressOf(*currentDest))) valueType(*begin);
			}

			return currentDest;
		}
	};

	template<>
	struct UninitializedCopyImpl<true, true, false>
	{
		template<typename InputIterator, typename ForwardIterator>
		static ForwardIterator Impl(InputIterator begin, InputIterator end, ForwardIterator dest)
		{
			typedef typename std::iterator_traits<ForwardIterator>::value_type valueType;
			
			for (; begin != end; ++begin, ++dest)
			{
				memmove(AddressOf(*dest), AddressOf(*begin), sizeof(valueType));
			}

			return dest;
		}
	};

	template<>
	struct UninitializedCopyImpl<true, true, true>
	{
		template<typename InputIterator, typename ForwardIterator>
		static ForwardIterator Impl(InputIterator begin, InputIterator end, ForwardIterator dest)
		{
			typedef typename std::iterator_traits<ForwardIterator>::value_type valueType;

			if (begin == end)
			{
				return dest;
			}

			auto count = (end - begin);
			memmove(AddressOf(*dest), AddressOf(*begin), sizeof(valueType) * count);
			return dest + count;
		}
	};
}

// Uninitialized copy
/*
	Copies a source range to a destination by copy-constructing the destination with
	the source values.

	Returns the end of the destination range
*/

template<typename InputIterator, typename ForwardIterator>
inline ForwardIterator UninitializedCopy(InputIterator begin, InputIterator end, ForwardIterator result)
{
	typedef typename std::iterator_traits<InputIterator>::iterator_category IIC;
	typedef typename std::iterator_traits<ForwardIterator>::iterator_category OIC;
	typedef typename std::iterator_traits<InputIterator>::value_type valueTypeInput;
	typedef typename std::iterator_traits<ForwardIterator>::value_type valueTypeOutput;

	constexpr bool isTriviallyCopyable = std::is_same<valueTypeInput, valueTypeOutput>::value&& std::is_trivially_copyable<valueTypeOutput>::value;
	constexpr bool isInputIteratorReferenceAddressable = std::is_convertible<typename std::add_lvalue_reference<valueTypeInput>::type, typename std::iterator_traits<InputIterator>::reference>::value;
	constexpr bool areIteratorsContiguous = (std::is_pointer<InputIterator>::value || std::_Iterator_is_contiguous<IIC>) &&
										   (std::is_pointer<ForwardIterator>::value || std::_Iterator_is_contiguous<OIC>);

	return Internal::UninitializedCopyImpl<isTriviallyCopyable, isInputIteratorReferenceAddressable, areIteratorsContiguous>::Impl(begin, end, result);
}

template<typename Begin, typename End, typename Result>
inline Result UninitializedCopyPtr(Begin begin, End end, Result result)
{
	return UninitializedCopy(begin, end, result);
}

// Uninitialized move
namespace Internal
{
	template<typename InputIterator, typename ForwardIterator>
	inline ForwardIterator UninitializedMoveImpl(InputIterator begin, InputIterator end, ForwardIterator dest, std::true_type)
	{
		return std::copy(begin, end, dest);
	}

	template<typename InputIterator, typename ForwardIterator>
	inline ForwardIterator UninitializedMoveImpl(InputIterator begin, InputIterator end, ForwardIterator dest, std::false_type)
	{
		typedef typename std::iterator_traits<ForwardIterator>::value_type ValueType;
		ForwardIterator currentDest(dest);

		for (; begin != end; ++begin, ++currentDest)
		{
			::new((void*)AddressOf(*currentDest)) ValueType(std::move(*begin));
		}

		return currentDest;
	}
}

template<typename Begin, typename End, typename Result>
inline Result UninitializedMovePtr(Begin first, End last, Result dest)
{
	typedef typename std::iterator_traits<GenericIterator<Result, void>>::value_type ValueType;
	const GenericIterator<Result, void> i(Internal::UninitializedMoveImpl(GenericIterator<Begin, void>(first),
																		  GenericIterator<End, void>(last),
																		  GenericIterator<Result, void>(dest),
																		  std::is_trivially_copyable<ValueType>()));

	return i.base();
}

// Uninitialized Value Construct with count
template<class ForwardIterator, class Count>
inline ForwardIterator UninitializedValueConstructCount(ForwardIterator first, Count count)
{
	typedef typename std::iterator_traits<ForwardIterator>::value_type valueType;
	ForwardIterator currentDest(first);

	for (; count > 0; --count, ++currentDest)
	{
		::new(Internal::AddressOf(*currentDest)) valueType();
	}

	return currentDest;
}

// Uninitialized Construct Fill with count
namespace Internal
{
	template<typename ForwardIterator, typename Count, typename T>
	inline void UnintinializedFillCountImpl(ForwardIterator first, Count count, const T& value, std::true_type)
	{
		std::fill(first, count, value);
	}

	template<typename ForwardIterator, typename Count, typename T>
	inline void UnintinializedFillCountImpl(ForwardIterator first, Count count, const T& value, std::false_type)
	{
		typedef typename std::iterator_traits<ForwardIterator>::value_type valueType;
		ForwardIterator currentDest(first);

		for (; count > 0; --count, ++currentDest)
		{
			::new(Internal::AddressOf(*currentDest)) valueType(value);
		}
	}
}

template<typename ForwardIterator, typename Count, typename T>
inline void UninitializedConstructFillCount(ForwardIterator first, Count count, const T& value)
{
	typedef typename std::iterator_traits<ForwardIterator>::value_type valueType;
	Internal::UnintinializedFillCountImpl(first, count, value, std::is_trivially_copy_assignable<valueType>());
}

template<typename T, typename Count>
inline void UninitializedConstructFillCountPtr(T* first, Count count, const T& value)
{
	T* currentDest = first;
	for (; count > 0; --count, ++currentDest)
	{
		:: new(Internal::AddressOf(*currentDest)) T(value);
	}
}

// Destruct
namespace Internal
{
	// Type has a trivial destructor (We do not need to call destructor)
	template<typename ForwardIterator>
	inline void DestructImpl(ForwardIterator, ForwardIterator, std::true_type)
	{ }

	template<typename ForwardIterator>
	inline void DestructImpl(ForwardIterator begin, ForwardIterator end, std::false_type)
	{
		typedef typename std::iterator_traits<ForwardIterator>::value_type valueType;

		for (; begin != end; ++begin)
		{
			(*begin).~valueType();
		}
	}
}

template<typename T>
inline void Destruct(T* ptr)
{
	VT_UNUSED(ptr);
	ptr->~T();
}

template<typename ForwardIterator>
inline void Destruct(ForwardIterator begin, ForwardIterator end)
{
	typedef typename std::iterator_traits<ForwardIterator>::value_type valueType;
	Internal::DestructImpl(begin, end, std::is_trivially_destructible<valueType>());
}
