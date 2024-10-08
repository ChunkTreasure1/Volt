#pragma once

#include <iterator>

template<typename Iterator, typename Container = void>
class GenericIterator
{
public:
	typedef typename std::iterator_traits<Iterator>::iterator_category iterator_category;
	typedef typename std::iterator_traits<Iterator>::value_type value_type;
	typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
	typedef typename std::iterator_traits<Iterator>::reference reference;
	typedef typename std::iterator_traits<Iterator>::pointer pointer;
	typedef Iterator iterator_type;
	typedef Container container_type;
	typedef GenericIterator<Iterator, Container> this_type;

	GenericIterator()
		: m_iterator(iterator_type())
	{ }

	explicit GenericIterator(const iterator_type& value)
		: m_iterator(value)
	{}

	template<typename Iterator2>
	GenericIterator(const GenericIterator<Iterator2, Container>& other)
		: m_iterator(other.base())
	{}

	reference operator*() const
	{
		return *m_iterator;
	}

	pointer operator->() const
	{
		return m_iterator;
	}

	this_type& operator++()
	{
		++m_iterator;
		return *this;
	}

	this_type operator++(int)
	{
		return this_type(m_iterator++);
	}

	this_type& operator--()
	{
		--m_iterator;
		return *this;
	}

	this_type operator--(int)
	{
		return this_type(m_iterator--);
	}

	reference operator[](const difference_type& n) const
	{
		return m_iterator[n];
	}

	this_type& operator+=(const difference_type& n)
	{
		m_iterator += n;
		return *this;
	}

	this_type operator+(const difference_type& n) const
	{
		return this_type(m_iterator + n);
	}

	this_type& operator-=(const difference_type& n)
	{
		m_iterator -= n;
		return *this;
	}

	this_type operator-(const difference_type& n) const
	{
		return this_type(m_iterator - n);
	}

	const iterator_type& base() const
	{
		return m_iterator;
	}

	this_type& operator=(const iterator_type& value)
	{ 
		m_iterator = value;
		return *this;
	}

protected:
	Iterator m_iterator;
};

template <typename IteratorL, typename IteratorR, typename Container>
inline bool operator==(const GenericIterator<IteratorL, Container>& lhs, const GenericIterator<IteratorR, Container>& rhs)
{
	return lhs.base() == rhs.base();
}

template <typename Iterator, typename Container>
inline bool operator==(const GenericIterator<Iterator, Container>& lhs, const GenericIterator<Iterator, Container>& rhs)
{
	return lhs.base() == rhs.base();
}

template <typename IteratorL, typename IteratorR, typename Container>
inline bool operator!=(const GenericIterator<IteratorL, Container>& lhs, const GenericIterator<IteratorR, Container>& rhs)
{
	return lhs.base() != rhs.base();
}

template <typename Iterator, typename Container>
inline bool operator!=(const GenericIterator<Iterator, Container>& lhs, const GenericIterator<Iterator, Container>& rhs)
{
	return lhs.base() != rhs.base();
}

template <typename IteratorL, typename IteratorR, typename Container>
inline bool operator<(const GenericIterator<IteratorL, Container>& lhs, const GenericIterator<IteratorR, Container>& rhs)
{
	return lhs.base() < rhs.base();
}

template <typename Iterator, typename Container>
inline bool operator<(const GenericIterator<Iterator, Container>& lhs, const GenericIterator<Iterator, Container>& rhs)
{
	return lhs.base() < rhs.base();
}

template <typename IteratorL, typename IteratorR, typename Container>
inline bool operator>(const GenericIterator<IteratorL, Container>& lhs, const GenericIterator<IteratorR, Container>& rhs)
{
	return lhs.base() > rhs.base();
}

template <typename Iterator, typename Container>
inline bool operator>(const GenericIterator<Iterator, Container>& lhs, const GenericIterator<Iterator, Container>& rhs)
{
	return lhs.base() > rhs.base();
}

template <typename IteratorL, typename IteratorR, typename Container>
inline bool operator<=(const GenericIterator<IteratorL, Container>& lhs, const GenericIterator<IteratorR, Container>& rhs)
{
	return lhs.base() <= rhs.base();
}

template <typename Iterator, typename Container>
inline bool operator<=(const GenericIterator<Iterator, Container>& lhs, const GenericIterator<Iterator, Container>& rhs)
{
	return lhs.base() <= rhs.base();
}

template <typename IteratorL, typename IteratorR, typename Container>
inline bool operator>=(const GenericIterator<IteratorL, Container>& lhs, const GenericIterator<IteratorR, Container>& rhs)
{
	return lhs.base() >= rhs.base();
}

template <typename Iterator, typename Container>
inline bool operator>=(const GenericIterator<Iterator, Container>& lhs, const GenericIterator<Iterator, Container>& rhs)
{
	return lhs.base() >= rhs.base();
}

template <typename IteratorL, typename IteratorR, typename Container>
inline typename GenericIterator<IteratorL, Container>::difference_type
operator-(const GenericIterator<IteratorL, Container>& lhs, const GenericIterator<IteratorR, Container>& rhs)
{
	return lhs.base() - rhs.base();
}

template <typename Iterator, typename Container>
inline GenericIterator<Iterator, Container>
operator+(typename GenericIterator<Iterator, Container>::difference_type n, const GenericIterator<Iterator, Container>& x)
{
	return GenericIterator<Iterator, Container>(x.base() + n);
}
