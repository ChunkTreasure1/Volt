#pragma once

#include "CoreUtilities/CompilerTraits.h"
#include "CoreUtilities/TypeTraits.h"
#include "CoreUtilities/Memory.h"
#include "CoreUtilities/VoltAssert.h"

#include <cstdint>
#include <algorithm>
#include <initializer_list>
#include <iterator>

template<typename T>
class Vector
{
public:
	typedef T value_type;
	typedef size_t size_type;
	typedef T* iterator;
	typedef const T* const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	inline static constexpr size_type npos = (size_type)-1;

	constexpr Vector() noexcept = default;
	constexpr Vector(size_type count) noexcept;
	constexpr Vector(size_type count, const T& value) noexcept;
	constexpr Vector(const Vector<T>& other) noexcept;
	constexpr Vector(Vector<T>&& other) noexcept;
	constexpr Vector(std::initializer_list<T> initList) noexcept;
	template<typename InputIterator> constexpr Vector(InputIterator begin, InputIterator end) noexcept;

	constexpr ~Vector();

	constexpr Vector<T>& operator=(const Vector<T>& other) noexcept;
	constexpr Vector<T>& operator=(std::initializer_list<T> initList) noexcept;
	constexpr Vector<T>& operator=(Vector<T>&& other) noexcept;

	constexpr void swap(Vector<T>& other);

	constexpr void assign(size_type count, const value_type& value);

	template<typename InputIterator>
	constexpr void assign(InputIterator first, InputIterator last);

	constexpr void assign(std::initializer_list<value_type> initList);

	constexpr iterator append(const Vector<T>& other) noexcept;

	VT_NODISCARD constexpr iterator begin() noexcept;
	VT_NODISCARD constexpr const_iterator begin() const noexcept;
	VT_NODISCARD constexpr const_iterator cbegin() const noexcept;

	VT_NODISCARD constexpr iterator end() noexcept;
	VT_NODISCARD constexpr const_iterator end() const noexcept;
	VT_NODISCARD constexpr const_iterator cend() const noexcept;

	VT_NODISCARD constexpr reverse_iterator rbegin() noexcept;
	VT_NODISCARD constexpr const_reverse_iterator rbegin() const noexcept;
	VT_NODISCARD constexpr const_reverse_iterator crbegin() const noexcept;

	VT_NODISCARD constexpr reverse_iterator rend() noexcept;
	VT_NODISCARD constexpr const_reverse_iterator rend() const noexcept;
	VT_NODISCARD constexpr const_reverse_iterator crend() const noexcept;

	VT_NODISCARD constexpr bool empty() const noexcept;
	VT_NODISCARD constexpr size_type size() const noexcept;
	VT_NODISCARD constexpr size_type capacity() const noexcept;

	constexpr void resize(size_type count, const value_type& value);
	constexpr void resize(size_type count);
	constexpr void resize_uninitialized(size_type count);
	constexpr void reserve(size_type count);
	constexpr void set_capacity(size_type count = npos);
	constexpr void shrink_to_fit();

	VT_NODISCARD constexpr value_type* data() noexcept;
	VT_NODISCARD constexpr const value_type* data() const noexcept;

	VT_NODISCARD constexpr value_type& operator[](size_type position);
	VT_NODISCARD constexpr const value_type& operator[](size_type position) const;

	VT_NODISCARD constexpr value_type& at(size_type position);
	VT_NODISCARD constexpr const value_type& at(size_type position) const;

	VT_NODISCARD constexpr value_type& front();
	VT_NODISCARD constexpr const value_type& front() const;

	VT_NODISCARD constexpr value_type& back();
	VT_NODISCARD constexpr const value_type& back() const;

	constexpr void push_back(const value_type& value);
	constexpr value_type& push_back();
	constexpr void push_back(value_type&& value);
	constexpr void pop_back();

	template<typename... Args>
	constexpr iterator emplace(const_iterator position, Args&&... args);

	template<typename... Args>
	constexpr value_type& emplace_back(Args&&... args);

	constexpr iterator insert(const_iterator position, const value_type& value);
	constexpr iterator insert(const_iterator position, size_type count, const value_type& value);
	constexpr iterator insert(const_iterator position, value_type&& value);
	constexpr iterator insert(const_iterator position, std::initializer_list<value_type> initList);

	template<typename InputIterator>
	constexpr iterator insert(const_iterator position, InputIterator first, InputIterator last);

	constexpr iterator erase_first(const T& value);
	constexpr iterator erase_first_unsorted(const T& value);
	constexpr reverse_iterator erase_last(const T& value);
	constexpr reverse_iterator erase_last_unsorted(const T& value);

	constexpr iterator erase(const_iterator position);
	constexpr iterator erase(const_iterator first, const_iterator last);
	constexpr iterator erase_unsorted(const_iterator position);

	constexpr reverse_iterator erase(const_reverse_iterator position);
	constexpr reverse_iterator erase(const_reverse_iterator first, const_reverse_iterator last);
	constexpr reverse_iterator erase_unsorted(const_reverse_iterator position);

	template<typename PredicateFunctor>
	constexpr void erase_with_predicate(PredicateFunctor functor);

	constexpr void clear() noexcept;

protected:
	template<bool move> struct ShouldMoveOrCopyTag {};
	using ShouldCopyTag = ShouldMoveOrCopyTag<false>;
	using ShouldMoveTag = ShouldMoveOrCopyTag<true>;

	template<typename ForwardIterator>
	value_type* Reallocate(size_type count, ForwardIterator first, ForwardIterator last, ShouldCopyTag);

	template<typename ForwardIterator>
	value_type* Reallocate(size_type count, ForwardIterator first, ForwardIterator last, ShouldMoveTag);

	template<typename Integer>
	void Initialize(Integer count, Integer value, std::true_type);

	template<typename InputIterator>
	void Initialize(InputIterator begin, InputIterator end, std::false_type);

	template<typename InputIterator>
	void InitializeFromIterator(InputIterator begin, InputIterator end, std::input_iterator_tag);

	template<typename ForwardIterator>
	void InitializeFromIterator(ForwardIterator begin, ForwardIterator end, std::forward_iterator_tag);

	template<typename Integer, bool move>
	void assign(Integer n, Integer value, std::true_type);

	template<typename InputIterator, bool move>
	void assign(InputIterator begin, InputIterator end, std::false_type);

	void AssignValues(size_type count, const T& value);

	template <typename InputIterator, bool move>
	void AssignFromIterator(InputIterator begin, InputIterator end, std::input_iterator_tag);

	template <typename RandomAccessIterator, bool move>
	void AssignFromIterator(RandomAccessIterator begin, RandomAccessIterator end, std::random_access_iterator_tag);

	template <typename Integer>
	void insert(const_iterator position, Integer count, Integer value, std::true_type);

	template <typename InputIterator>
	void insert(const_iterator position, InputIterator first, InputIterator last, std::false_type);

	template <typename InputIterator>
	void InsertFromIterator(const_iterator position, InputIterator first, InputIterator last, std::input_iterator_tag);

	template <typename BidirectionalIterator>
	void InsertFromIterator(const_iterator position, BidirectionalIterator first, BidirectionalIterator last, std::bidirectional_iterator_tag);

	void InsertValues(const_iterator position, size_type count, const value_type& value);

	template<typename... Args>
	void InsertValue(const_iterator position, Args&&... args);

	template<typename... Args>
	void InsertValueAtEnd(Args&&... args);

	void InsertValuesAtEnd(size_type count); // Default constructs count values
	void InsertValuesAtEnd(size_type count, const value_type& value);

	void ClearCapacity();

	size_type GetNewCapacity(size_type currentCapacity);
	void Grow(size_type count);

	void InitializeAllocation(size_type count);
	T* Allocate(size_type count);

private:
	T* m_ptrBegin = nullptr;
	T* m_ptrEnd = nullptr;
	T* m_ptrCapacity = nullptr;
};

template<typename T>
inline constexpr Vector<T>::Vector(size_type count) noexcept
{
	InitializeAllocation(count);
	UninitializedValueConstructCount(m_ptrBegin, count);
	m_ptrEnd = m_ptrBegin + count;
}

template<typename T>
inline constexpr Vector<T>::Vector(size_type count, const T& value) noexcept
{
	InitializeAllocation(count);
	UninitializedConstructFillCountPtr(m_ptrBegin, count, value);
	m_ptrEnd = m_ptrBegin + count;
}

template<typename T>
inline constexpr Vector<T>::Vector(const Vector<T>& other) noexcept
{
	InitializeAllocation(other.size());
	m_ptrEnd = UninitializedCopyPtr(other.m_ptrBegin, other.m_ptrEnd, m_ptrBegin);
}

template<typename T>
inline constexpr Vector<T>::Vector(Vector<T>&& other) noexcept
{
	swap(other);
}

template<typename T>
inline constexpr Vector<T>::Vector(std::initializer_list<T> initList) noexcept
{
	Initialize(initList.begin(), initList.end(), std::false_type());
}

template<typename T>
template<typename InputIterator>
inline constexpr Vector<T>::Vector(InputIterator begin, InputIterator end) noexcept
{
	Initialize(begin, end, std::is_integral<InputIterator>());
}

template<typename T>
inline constexpr Vector<T>::~Vector()
{
	Destruct(m_ptrBegin, m_ptrEnd);

	if (m_ptrBegin)
	{
		Internal::Free(m_ptrBegin);
	}
}

template<typename T>
inline constexpr Vector<T>& Vector<T>::operator=(const Vector<T>& rhs) noexcept
{
	if (this != &rhs)
	{
		assign<const_iterator, false>(rhs.begin(), rhs.end(), std::false_type());
	}

	return *this;
}

template<typename T>
inline constexpr Vector<T>& Vector<T>::operator=(std::initializer_list<T> initList) noexcept
{
	typedef typename std::initializer_list<value_type>::iterator InputIterator;
	typedef typename std::iterator_traits<InputIterator>::iterator_category IC;
	AssignFromIterator<InputIterator, false>(initList.begin(), initList.end(), IC());
	return *this;
}

template<typename T>
inline constexpr Vector<T>& Vector<T>::operator=(Vector<T>&& other) noexcept
{
	if (this != &other)
	{
		ClearCapacity();
		swap(other);
	}

	return *this;
}

template<typename T>
inline constexpr void Vector<T>::swap(Vector<T>& other)
{
	std::swap(m_ptrBegin, other.m_ptrBegin);
	std::swap(m_ptrEnd, other.m_ptrEnd);
	std::swap(m_ptrCapacity, other.m_ptrCapacity);
}

template<typename T>
inline constexpr void Vector<T>::assign(size_type count, const value_type& value)
{
	AssignValues(count, value);
}

template<typename T>
template<typename InputIterator>
inline constexpr void Vector<T>::assign(InputIterator first, InputIterator last)
{
	assign<InputIterator, false>(first, last, std::is_integral<InputIterator>());
}

template<typename T>
inline constexpr void Vector<T>::assign(std::initializer_list<value_type> initList)
{
	typedef typename std::initializer_list<value_type>::iterator InputIterator;
	typedef typename std::iterator_traits<InputIterator>::iterator_category IC;

	AssignFromIterator<InputIterator, false>(initList.begin(), initList.end(), IC());
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::append(const Vector<T>& other) noexcept
{
	return insert(end(), other.begin(), other.end());
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::begin() noexcept
{
	return m_ptrBegin;
}

template<typename T>
inline constexpr Vector<T>::const_iterator Vector<T>::begin() const noexcept
{
	return m_ptrBegin;
}

template<typename T>
inline constexpr Vector<T>::const_iterator Vector<T>::cbegin() const noexcept
{
	return m_ptrBegin;
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::end() noexcept
{
	return m_ptrEnd;
}

template<typename T>
inline constexpr Vector<T>::const_iterator Vector<T>::end() const noexcept
{
	return m_ptrEnd;
}

template<typename T>
inline constexpr Vector<T>::const_iterator Vector<T>::cend() const noexcept
{
	return m_ptrEnd;
}

template<typename T>
inline constexpr Vector<T>::reverse_iterator Vector<T>::rbegin() noexcept
{
	return std::reverse_iterator(m_ptrEnd);
}

template<typename T>
inline constexpr Vector<T>::const_reverse_iterator Vector<T>::rbegin() const noexcept
{
	return std::reverse_iterator(m_ptrEnd);
}

template<typename T>
inline constexpr Vector<T>::const_reverse_iterator Vector<T>::crbegin() const noexcept
{
	return std::reverse_iterator(m_ptrEnd);
}

template<typename T>
inline constexpr Vector<T>::reverse_iterator Vector<T>::rend() noexcept
{
	return std::reverse_iterator(m_ptrBegin);
}

template<typename T>
inline constexpr Vector<T>::const_reverse_iterator Vector<T>::rend() const noexcept
{
	return std::reverse_iterator(m_ptrBegin);
}

template<typename T>
inline constexpr Vector<T>::const_reverse_iterator Vector<T>::crend() const noexcept
{
	return std::reverse_iterator(m_ptrBegin);
}

template<typename T>
inline constexpr bool Vector<T>::empty() const noexcept
{
	return (m_ptrBegin == m_ptrEnd);
}

template<typename T>
inline constexpr Vector<T>::size_type Vector<T>::size() const noexcept
{
	return static_cast<size_type>(m_ptrEnd - m_ptrBegin);
}

template<typename T>
inline constexpr Vector<T>::size_type Vector<T>::capacity() const noexcept
{
	return static_cast<size_type>(m_ptrCapacity - m_ptrBegin);
}

template<typename T>
inline constexpr void Vector<T>::resize(size_type count, const value_type& value)
{
	if (count > static_cast<size_type>(m_ptrEnd - m_ptrBegin))
	{
		InsertValuesAtEnd(count - (static_cast<size_type>(m_ptrEnd - m_ptrBegin)), value);
	}
	else
	{
		Destruct(m_ptrBegin + count, m_ptrEnd);
		m_ptrEnd = m_ptrBegin + count;
	}
}

template<typename T>
inline constexpr void Vector<T>::resize(size_type count)
{
	if (count > static_cast<size_type>(m_ptrEnd - m_ptrBegin))
	{
		InsertValuesAtEnd(count - (static_cast<size_type>(m_ptrEnd - m_ptrBegin)));
	}
	else
	{
		Destruct(m_ptrBegin + count, m_ptrEnd);
		m_ptrEnd = m_ptrBegin + count;
	}
}

template<typename T>
inline constexpr void Vector<T>::resize_uninitialized(size_type count)
{
	static_assert(std::is_trivially_destructible_v<T>);

	if (count > static_cast<size_type>(m_ptrCapacity - m_ptrBegin))
	{
		const size_type prevCount = static_cast<size_type>(m_ptrEnd - m_ptrBegin);
		const size_type growCount = GetNewCapacity(prevCount);
		const size_type newCount = std::max(growCount, prevCount + count);

		Grow(newCount);
	}

	m_ptrEnd = m_ptrBegin + count;
}

template<typename T>
inline constexpr void Vector<T>::reserve(size_type count)
{
	if (count > static_cast<size_type>(m_ptrCapacity - m_ptrBegin))
	{
		Grow(count);
	}
}

template<typename T>
inline constexpr void Vector<T>::set_capacity(size_type count)
{
	if ((count == npos) || (count <= static_cast<size_type>(m_ptrEnd - m_ptrBegin)))
	{
		if (count == 0)
		{
			clear();
		}
		else if (count < static_cast<size_type>(m_ptrEnd - m_ptrBegin))
		{
			resize(count);
		}

		shrink_to_fit();
	}
	else
	{
		value_type* const newData = Reallocate(count, m_ptrBegin, m_ptrEnd, ShouldMoveTag());
		Destruct(m_ptrBegin, m_ptrEnd);
		Internal::Free(m_ptrBegin);

		const ptrdiff_t prevCount = m_ptrEnd - m_ptrBegin;
		m_ptrBegin = newData;
		m_ptrEnd = newData + prevCount;
		m_ptrCapacity = m_ptrBegin + count;
	}
}

template<typename T>
inline constexpr void Vector<T>::shrink_to_fit()
{
	Vector<T> temp = Vector<T>(std::move_iterator<iterator>(begin()), std::move_iterator<iterator>(end()));
	swap(temp);
}

template<typename T>
inline constexpr Vector<T>::value_type* Vector<T>::data() noexcept
{
	return m_ptrBegin;
}

template<typename T>
inline constexpr const Vector<T>::value_type* Vector<T>::data() const noexcept
{
	return m_ptrBegin;
}

template<typename T>
inline constexpr Vector<T>::value_type& Vector<T>::operator[](size_type position)
{
	VT_ASSERT_MSG(position < static_cast<size_type>(m_ptrEnd - m_ptrBegin), "Vector::operator[] - Out of range!");
	return *(m_ptrBegin + position);
}

template<typename T>
inline constexpr const Vector<T>::value_type& Vector<T>::operator[](size_type position) const
{
	VT_ASSERT_MSG(position < static_cast<size_type>(m_ptrEnd - m_ptrBegin), "Vector::operator[] - Out of range!");
	return *(m_ptrBegin + position);
}

template<typename T>
inline constexpr Vector<T>::value_type& Vector<T>::at(size_type position)
{
	VT_ASSERT_MSG(position < static_cast<size_type>(m_ptrEnd - m_ptrBegin), "Vector::At - Out of range!");
	return *(m_ptrBegin + position);
}

template<typename T>
inline constexpr const Vector<T>::value_type& Vector<T>::at(size_type position) const
{
	VT_ASSERT_MSG(position < static_cast<size_type>(m_ptrEnd - m_ptrBegin), "Vector::At - Out of range!");
	return *(m_ptrBegin + position);
}

template<typename T>
inline constexpr Vector<T>::value_type& Vector<T>::front()
{
	VT_ASSERT_MSG((m_ptrBegin != nullptr) && (m_ptrEnd > m_ptrBegin), "Vector::Front - Empty Vector!");
	return *m_ptrBegin;
}

template<typename T>
inline constexpr const Vector<T>::value_type& Vector<T>::front() const
{
	VT_ASSERT_MSG((m_ptrBegin != nullptr) && (m_ptrEnd > m_ptrBegin), "Vector::Front - Empty Vector!");
	return *m_ptrBegin;
}

template<typename T>
inline constexpr Vector<T>::value_type& Vector<T>::back()
{
	VT_ASSERT_MSG((m_ptrBegin != nullptr) && (m_ptrEnd > m_ptrBegin), "Vector::Back - Empty Vector!");
	return *(m_ptrEnd - 1);
}

template<typename T>
inline constexpr const Vector<T>::value_type& Vector<T>::back() const
{
	VT_ASSERT_MSG((m_ptrBegin != nullptr) && (m_ptrEnd > m_ptrBegin), "Vector::Back - Empty Vector!");
	return *(m_ptrEnd - 1);
}

template<typename T>
inline constexpr void Vector<T>::push_back(const value_type& value)
{
	if (m_ptrEnd < m_ptrCapacity)
	{
		::new((void*)m_ptrEnd++) T(value);
	}
	else
	{
		InsertValueAtEnd(std::move(value));
	}
}

template<typename T>
inline constexpr Vector<T>::value_type& Vector<T>::push_back()
{
	if (m_ptrEnd < m_ptrCapacity)
	{
		::new(static_cast<void*>(m_ptrEnd++)) value_type();
	}
	else
	{
		InsertValueAtEnd(value_type());
	}

	return *(m_ptrEnd - 1);
}

template<typename T>
inline constexpr void Vector<T>::push_back(value_type&& value)
{
	if (m_ptrEnd < m_ptrCapacity)
	{
		::new(static_cast<void*>(m_ptrEnd++)) value_type(std::move(value));
	}
	else
	{
		InsertValueAtEnd(std::move(value));
	}
}

template<typename T>
inline constexpr void Vector<T>::pop_back()
{
	VT_ASSERT_MSG(m_ptrEnd != m_ptrBegin, "Vector::PopBack - Empty Vector!");

	--m_ptrEnd;
	m_ptrEnd->~value_type();
}

template<typename T>
template<typename ...Args>
inline constexpr Vector<T>::iterator Vector<T>::emplace(const_iterator position, Args&& ...args)
{
	const ptrdiff_t count = position - m_ptrBegin;

	if ((m_ptrEnd == m_ptrCapacity) || (position != m_ptrEnd))
	{
		InsertValue(position, std::forward<Args>(args)...);
	}
	else
	{
		::new(static_cast<void*>(m_ptrEnd)) value_type(std::forward<Args>(args)...);
		++m_ptrEnd;
	}

	return m_ptrBegin + count;
}

template<typename T>
template<typename ...Args>
inline constexpr Vector<T>::value_type& Vector<T>::emplace_back(Args && ...args)
{
	if (m_ptrEnd < m_ptrCapacity)
	{
		::new(static_cast<void*>(m_ptrEnd)) value_type(std::forward<Args>(args)...);
		++m_ptrEnd;
	}
	else
	{
		InsertValueAtEnd(std::forward<Args>(args)...);
	}

	return back();
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::insert(const_iterator position, const value_type& value)
{
	VT_ASSERT_MSG((position >= m_ptrBegin) || (position <= m_ptrEnd), "Vector::Insert - Invalid position!");

	const ptrdiff_t count = position - m_ptrBegin;

	if ((m_ptrEnd == m_ptrCapacity) || (position != m_ptrEnd))
	{
		InsertValue(position, value);
	}
	else
	{
		::new(static_cast<void*>(m_ptrEnd)) value_type(value);
		++m_ptrEnd;
	}

	return m_ptrBegin + count;
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::insert(const_iterator position, value_type&& value)
{
	return emplace(position, std::move(value));
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::insert(const_iterator position, size_type count, const value_type& value)
{
	const ptrdiff_t p = position - m_ptrBegin;
	InsertValues(position, count, value);

	return m_ptrBegin + p;
}

template<typename T>
template<typename InputIterator>
inline constexpr Vector<T>::iterator Vector<T>::insert(const_iterator position, InputIterator first, InputIterator last)
{
	const ptrdiff_t p = position - m_ptrBegin;
	insert(position, first, last, std::is_integral<InputIterator>());

	return m_ptrBegin + p;
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::insert(const_iterator position, std::initializer_list<value_type> initList)
{
	const ptrdiff_t p = position - m_ptrBegin;
	insert(position, initList.begin(), initList.end(), std::false_type());

	return m_ptrBegin + p;
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::erase_first(const T& value)
{
	static_assert(HasEqualityV<T>, "T must be comparable!");

	iterator it = std::find(begin(), end(), value);

	if (it != end())
	{
		return erase(it);
	}
	else
	{
		return it;
	}
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::erase_first_unsorted(const T& value)
{
	static_assert(HasEqualityV<T>, "T must be comparable!");

	iterator it = std::find(begin(), end(), value);

	if (it != end())
	{
		return erase_unsorted(it);
	}
	else
	{
		return it;
	}
}

template<typename T>
inline constexpr Vector<T>::reverse_iterator Vector<T>::erase_last(const T& value)
{
	static_assert(HasEqualityV<T>, "T must be comparable!");

	reverse_iterator it = std::find(rbegin(), rend(), value);
	if (it != rend())
	{
		return erase(it);
	}
	else
	{
		return it;
	}
}

template<typename T>
inline constexpr Vector<T>::reverse_iterator Vector<T>::erase_last_unsorted(const T& value)
{
	reverse_iterator it = std::find(rbegin(), rend(), value);
	if (it != rend())
	{
		return erase_unsorted(it);
	}
	else
	{
		return it;
	}
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::erase(const_iterator position)
{
	VT_ASSERT_MSG((position >= m_ptrBegin) && (position < m_ptrEnd), "Vector::Erase - Invalid position!");

	iterator destPosition = const_cast<value_type*>(position);

	if ((position + 1) < m_ptrEnd)
	{
		std::move(destPosition + 1, m_ptrEnd, destPosition);
	}

	--m_ptrEnd;
	m_ptrEnd->~value_type();
	return destPosition;
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::erase(const_iterator first, const_iterator last)
{
	VT_ASSERT_MSG((first >= m_ptrBegin) && (first < m_ptrEnd) && (last > m_ptrBegin) && (last <= m_ptrEnd) && (last > first), "Vector::Erase - Invalid position!");

	if (first != last)
	{
		const iterator position = const_cast<T*>(std::move(const_cast<T*>(last), const_cast<T*>(m_ptrEnd), const_cast<T*>(first)));
		Destruct(position, m_ptrEnd);
		m_ptrEnd -= (last - first);
	}

	return const_cast<T*>(first);
}

template<typename T>
inline constexpr Vector<T>::iterator Vector<T>::erase_unsorted(const_iterator position)
{
	VT_ASSERT_MSG((position >= m_ptrBegin) && (position < m_ptrEnd), "Vector::EraseUnsorted - Invalid position!");

	iterator destPosition = const_cast<value_type*>(position);
	*destPosition = std::move(*(m_ptrEnd - 1));

	--m_ptrEnd;
	m_ptrEnd->~value_type();

	return destPosition;
}

template<typename T>
inline constexpr Vector<T>::reverse_iterator Vector<T>::erase(const_reverse_iterator position)
{
	return reverse_iterator(erase((++position).base()));
}

template<typename T>
inline constexpr Vector<T>::reverse_iterator Vector<T>::erase(const_reverse_iterator first, const_reverse_iterator last)
{
	return reverse_iterator(erase(last.base(), first.base()));
}

template<typename T>
inline constexpr Vector<T>::reverse_iterator Vector<T>::erase_unsorted(const_reverse_iterator position)
{
	return reverse_iterator(erase_unsorted((++position).base()));
}

template<typename T>
template<typename PredicateFunctor>
inline constexpr void Vector<T>::erase_with_predicate(PredicateFunctor functor)
{
	for (auto it = rbegin(); it != rend(); --it)
	{
		if (functor(*it))
		{
			erase(it);
		}
	}
}

template<typename T>
inline constexpr void Vector<T>::clear() noexcept
{
	Destruct(m_ptrBegin, m_ptrEnd);
	m_ptrEnd = m_ptrBegin;
}

template<typename T>
inline void Vector<T>::InsertValuesAtEnd(size_type count, const value_type& value)
{
	if (count > static_cast<size_type>(m_ptrCapacity - m_ptrEnd))
	{
		const size_type prevCount = static_cast<size_type>(m_ptrEnd - m_ptrBegin);
		const size_type growCount = GetNewCapacity(prevCount);
		const size_type newCount = std::max(growCount, prevCount + count);
		value_type* const newData = Allocate(newCount);

		value_type* newEnd = UninitializedMovePtr(m_ptrBegin, m_ptrEnd, newData);

		UninitializedConstructFillCountPtr(newEnd, count, value);
		newEnd += count;

		Destruct(m_ptrBegin, m_ptrEnd);
		Internal::Free(m_ptrBegin);

		m_ptrBegin = newData;
		m_ptrEnd = newEnd;
		m_ptrCapacity = newData + newCount;
	}
	else
	{
		UninitializedConstructFillCountPtr(m_ptrEnd, count, value);
		m_ptrEnd += count;
	}
}

template<typename T>
inline void Vector<T>::InsertValuesAtEnd(size_type count)
{
	if (count > static_cast<size_type>(m_ptrCapacity - m_ptrEnd))
	{
		const size_type prevCount = static_cast<size_type>(m_ptrEnd - m_ptrBegin);
		const size_type growCount = GetNewCapacity(prevCount);
		const size_type newCount = std::max(growCount, prevCount + count);
		value_type* const newData = Allocate(newCount);

		value_type* newEnd = UninitializedMovePtr(m_ptrBegin, m_ptrEnd, newData);

		UninitializedValueConstructCount(newEnd, count);
		newEnd += count;

		Destruct(m_ptrBegin, m_ptrEnd);
		Internal::Free(m_ptrBegin);

		m_ptrBegin = newData;
		m_ptrEnd = newEnd;
		m_ptrCapacity = newData + newCount;
	}
	else
	{
		UninitializedValueConstructCount(m_ptrEnd, count);
		m_ptrEnd += count;
	}
}

template<typename T>
inline void Vector<T>::ClearCapacity()
{
	clear();
	Vector<T> temp(std::move(*this));
	swap(temp);
}

template<typename T>
inline void Vector<T>::Grow(size_type count)
{
	value_type* const newData = Allocate(count);
	value_type* newEnd = UninitializedMovePtr(m_ptrBegin, m_ptrEnd, newData);

	Destruct(m_ptrBegin, m_ptrEnd);
	Internal::Free(m_ptrBegin);

	m_ptrBegin = newData;
	m_ptrEnd = newEnd;
	m_ptrCapacity = newData + count;
}

template<typename T>
inline Vector<T>::size_type Vector<T>::GetNewCapacity(size_type currentCapacity)
{
	return (currentCapacity > 0) ? (2 * currentCapacity) : 1;
}

template<typename T>
inline void Vector<T>::InitializeAllocation(size_type count)
{
	m_ptrBegin = Allocate(count);
	m_ptrCapacity = m_ptrBegin + count;
}

template<typename T>
inline T* Vector<T>::Allocate(size_type count)
{
	if (count == 0)
	{
		return nullptr;
	}

	T* ptr = reinterpret_cast<T*>(Internal::Allocate(count * sizeof(T), alignof(T)));
	return ptr;
}

template<typename T>
template<typename ForwardIterator>
inline Vector<T>::value_type* Vector<T>::Reallocate(size_type count, ForwardIterator first, ForwardIterator last, ShouldCopyTag)
{
	value_type* const ptr = Allocate(count);
	UninitializedCopyPtr(first, last, ptr);
	return ptr;
}

template<typename T>
template<typename ForwardIterator>
inline Vector<T>::value_type* Vector<T>::Reallocate(size_type count, ForwardIterator first, ForwardIterator last, ShouldMoveTag)
{
	value_type* const ptr = Allocate(count);
	UninitializedMovePtr(first, last, ptr);
	return ptr;
}

template<typename T>
template<typename Integer>
inline void Vector<T>::Initialize(Integer count, Integer value, std::true_type)
{
	InitializeAllocation(static_cast<size_type>(count));
	m_ptrEnd = m_ptrCapacity;

	UninitializedConstructFillCountPtr<value_type, Integer>(m_ptrBegin, count, value);
}

template<typename T>
template<typename InputIterator>
inline void Vector<T>::Initialize(InputIterator begin, InputIterator end, std::false_type)
{
	typedef typename std::iterator_traits<InputIterator>::iterator_category IC;
	InitializeFromIterator(begin, end, IC());
}

template<typename T>
template<typename InputIterator>
inline void Vector<T>::InitializeFromIterator(InputIterator begin, InputIterator end, std::input_iterator_tag)
{
	for (; begin != end; ++begin)
	{
		push_back(*begin);
	}
}

template<typename T>
template<typename ForwardIterator>
inline void Vector<T>::InitializeFromIterator(ForwardIterator begin, ForwardIterator end, std::forward_iterator_tag)
{
	const size_type count = static_cast<size_type>(std::distance(begin, end));
	InitializeAllocation(count);
	m_ptrEnd = m_ptrCapacity;

	UninitializedCopyPtr(begin, end, m_ptrBegin);
}

template<typename T>
template<typename Integer, bool move>
inline void Vector<T>::assign(Integer n, Integer value, std::true_type)
{
	AssignValues(static_cast<size_type>(n), static_cast<T>(value));
}

template<typename T>
template<typename InputIterator, bool move>
inline void Vector<T>::assign(InputIterator begin, InputIterator end, std::false_type)
{
	typedef typename std::iterator_traits<InputIterator>::iterator_category IC;
	AssignFromIterator<InputIterator, move>(begin, end, IC());
}

template<typename T>
inline void Vector<T>::AssignValues(size_type count, const T& value)
{
	if (count > static_cast<size_type>(m_ptrCapacity - m_ptrBegin))
	{
		// If there isn't enough capacity, we must re allocate
		Vector<T> temp(count, value);
		swap(temp);
	}
	else if (count > static_cast<size_type>(m_ptrEnd - m_ptrBegin))
	{
		std::fill(m_ptrBegin, m_ptrEnd, value);
		UninitializedConstructFillCountPtr(m_ptrEnd, count - static_cast<size_type>(m_ptrEnd - m_ptrBegin), value);
		m_ptrEnd += count - static_cast<size_type>(m_ptrEnd - m_ptrBegin);
	}
	else
	{
		std::fill_n(m_ptrBegin, count, value);
		erase(m_ptrBegin + count, m_ptrEnd);
	}
}

template<typename T>
template<typename InputIterator, bool move>
inline void Vector<T>::AssignFromIterator(InputIterator begin, InputIterator end, std::input_iterator_tag)
{
	iterator position(m_ptrBegin);

	while ((position != m_ptrEnd) && (begin != end))
	{
		*position = *begin;
		++begin;
		++position;
	}

	if (begin == end)
	{
		erase(position, m_ptrEnd);
	}
	else
	{
		insert(m_ptrEnd, begin, end);
	}
}

template<typename T>
template<typename RandomAccessIterator, bool move>
inline void Vector<T>::AssignFromIterator(RandomAccessIterator begin, RandomAccessIterator end, std::random_access_iterator_tag)
{
	const size_type count = static_cast<size_type>(std::distance(begin, end));

	if (count > static_cast<size_type>(m_ptrCapacity - m_ptrBegin)) // count > capacity
	{
		value_type* const newData = Reallocate(count, begin, end, ShouldMoveOrCopyTag<move>());
		Destruct(m_ptrBegin, m_ptrEnd);
		Internal::Free(m_ptrBegin);

		m_ptrBegin = newData;
		m_ptrEnd = m_ptrBegin + count;
		m_ptrCapacity = m_ptrEnd;
	}
	else if (count <= static_cast<size_type>(m_ptrEnd - m_ptrBegin)) // count <= size
	{
		value_type* const newEnd = std::copy(begin, end, m_ptrBegin);
		Destruct(newEnd, m_ptrEnd);
		m_ptrEnd = newEnd;
	}
	else
	{
		RandomAccessIterator position = begin + (m_ptrEnd - m_ptrBegin);
		std::copy(begin, position, m_ptrBegin);
		m_ptrEnd = UninitializedCopyPtr(position, end, m_ptrEnd);
	}
}

template<typename T>
template<typename Integer>
inline void Vector<T>::insert(const_iterator position, Integer count, Integer value, std::true_type)
{
	InsertValues(position, static_cast<size_type>(count), static_cast<value_type>(value));
}

template<typename T>
template<typename InputIterator>
inline void Vector<T>::insert(const_iterator position, InputIterator first, InputIterator last, std::false_type)
{
	typedef typename std::iterator_traits<InputIterator>::iterator_category IC;
	InsertFromIterator(position, first, last, IC());
}

template<typename T>
template<typename InputIterator>
inline void Vector<T>::InsertFromIterator(const_iterator position, InputIterator first, InputIterator last, std::input_iterator_tag)
{
	for (; first != last; ++first, ++position)
	{
		position = insert(position, *first);
	}
}

template<typename T>
template<typename BidirectionalIterator>
inline void Vector<T>::InsertFromIterator(const_iterator position, BidirectionalIterator first, BidirectionalIterator last, std::bidirectional_iterator_tag)
{
	VT_ASSERT_MSG((position >= m_ptrBegin) && (position <= m_ptrEnd), "Vector::InsertFromIterator - Invalid position!");

	iterator destPosition = const_cast<value_type*>(position);

	if (first != last)
	{
		const size_type count = static_cast<size_type>(std::distance(first, last));

		if (count <= static_cast<size_type>(m_ptrCapacity - m_ptrEnd))
		{
			const size_type countExtra = static_cast<size_type>(m_ptrEnd - destPosition);

			if (count < countExtra)
			{
				UninitializedMovePtr(m_ptrEnd - count, m_ptrEnd, m_ptrEnd);
				std::move_backward(destPosition, m_ptrEnd - count, m_ptrEnd);
				std::copy(first, last, destPosition);
			}
			else
			{
				BidirectionalIterator tempIter = first;
				std::advance(tempIter, countExtra);
				UninitializedCopyPtr(tempIter, last, m_ptrEnd);
				UninitializedMovePtr(destPosition, m_ptrEnd, m_ptrEnd + count - countExtra);
				std::copy_backward(first, tempIter, destPosition + countExtra);
			}

			m_ptrEnd += count;
		}
		else
		{
			const size_type prevCount = static_cast<size_type>(m_ptrEnd - m_ptrBegin);
			const size_type growCount = GetNewCapacity(prevCount);
			const size_type newCount = growCount > (prevCount + count) ? growCount : (prevCount + count);
			value_type* const newData = Allocate(newCount);

			value_type* newEnd = UninitializedMovePtr(m_ptrBegin, destPosition, newData);
			newEnd = UninitializedCopyPtr(first, last, newEnd);
			newEnd = UninitializedMovePtr(destPosition, m_ptrEnd, newEnd);

			Destruct(m_ptrBegin, m_ptrEnd);
			Internal::Free(m_ptrBegin);

			m_ptrBegin = newData;
			m_ptrEnd = newEnd;
			m_ptrCapacity = newData + newCount;
		}
	}
}

template<typename T>
inline void Vector<T>::InsertValues(const_iterator position, size_type count, const value_type& value)
{
	VT_ASSERT_MSG((position >= m_ptrBegin) && (position <= m_ptrEnd), "Vector::InsertValues - Invalid position!");

	iterator destPosition = const_cast<value_type*>(position);
	if (count <= static_cast<size_type>(m_ptrCapacity - m_ptrEnd)) // count <= capacity
	{
		if (count > 0)
		{
			const value_type temp = value;
			const size_type insertPosition = static_cast<size_type>(m_ptrEnd - destPosition);

			if (count < insertPosition)
			{
				UninitializedMovePtr(m_ptrEnd - count, m_ptrEnd, m_ptrEnd);
				std::move_backward(destPosition, m_ptrEnd - count, m_ptrEnd);
				std::fill(destPosition, destPosition + count, temp);
			}
			else
			{
				UninitializedConstructFillCountPtr(m_ptrEnd, count - insertPosition, temp);
				UninitializedMovePtr(destPosition, m_ptrEnd, m_ptrEnd + count - insertPosition);
				std::fill(destPosition, m_ptrEnd, temp);
			}

			m_ptrEnd += count;
		}
	}
	else // count > capacity
	{
		const size_type prevCount = static_cast<size_type>(m_ptrEnd - m_ptrBegin);
		const size_type growCount = GetNewCapacity(prevCount);
		const size_type newCount = growCount > (prevCount + count) ? growCount : (prevCount + count);
		value_type* const newData = Allocate(newCount);

		value_type* newEnd = UninitializedMovePtr(m_ptrBegin, destPosition, newData);
		UninitializedConstructFillCountPtr(m_ptrEnd, count, value);
		newEnd = UninitializedMovePtr(destPosition, m_ptrEnd, newEnd + count);

		Destruct(m_ptrBegin, m_ptrEnd);
		Internal::Free(m_ptrBegin);

		m_ptrBegin = newData;
		m_ptrEnd = newEnd;
		m_ptrCapacity = newData + newCount;
	}
}

template<typename T>
template<typename ...Args>
inline void Vector<T>::InsertValue(const_iterator position, Args && ...args)
{
	VT_ASSERT_MSG((position >= m_ptrBegin) || (position <= m_ptrEnd), "Vector::InsertValue - Invalid position!");

	iterator destPosition = const_cast<value_type*>(position);

	if (m_ptrEnd != m_ptrCapacity) // Size < Capacity
	{
		VT_ASSERT(position < m_ptrEnd);

		value_type value(std::forward<Args>(args)...);

		::new(static_cast<void*>(m_ptrEnd)) value_type(std::move(*(m_ptrEnd - 1))); // m_ptrEnd is unitialized memory, thus we need to construct it the value
		std::move_backward(destPosition, m_ptrEnd - 1, m_ptrEnd);
		Destruct(destPosition);
		::new(static_cast<void*>(destPosition)) value_type(std::move(value));
		++m_ptrEnd;
	}
	else
	{
		const size_type insertPos = size_type(destPosition - m_ptrBegin);
		const size_type prevCount = size_type(m_ptrEnd - m_ptrBegin);
		const size_type newCount = GetNewCapacity(prevCount);
		value_type* const newData = Allocate(newCount);

		::new(static_cast<void*>(newData + insertPos)) value_type(std::forward<Args>(args)...);
		value_type* newEnd = UninitializedMovePtr(m_ptrBegin, destPosition, newData);
		newEnd = UninitializedMovePtr(destPosition, m_ptrEnd, ++newEnd);

		Destruct(m_ptrBegin, m_ptrEnd);
		Internal::Free(m_ptrBegin);

		m_ptrBegin = newData;
		m_ptrEnd = newEnd;
		m_ptrCapacity = newData + newCount;
	}
}

template<typename T>
template<typename ...Args>
inline void Vector<T>::InsertValueAtEnd(Args && ...args)
{
	const size_type prevCount = static_cast<size_type>(m_ptrEnd - m_ptrBegin);
	const size_type newCount = GetNewCapacity(prevCount);
	T* const newData = Allocate(newCount);

	T* newEnd = UninitializedMovePtr(m_ptrBegin, m_ptrEnd, newData);
	::new((void*)newEnd) T(std::forward<Args>(args)...);
	newEnd++;

	Destruct(m_ptrBegin, m_ptrEnd);
	Internal::Free(m_ptrBegin);

	m_ptrBegin = newData;
	m_ptrEnd = newEnd;
	m_ptrCapacity = newData + newCount;
}
