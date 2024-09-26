#pragma once

#include "CoreUtilities/Containers/Map.h"

#include <mutex>
#include <shared_mutex>

template<typename Key, typename Value>
class ThreadSafeMap : protected vt::map<Key, Value>
{
public:
	using WriteLock = std::unique_lock<std::shared_mutex>;
	using ReadLock = std::shared_lock<std::shared_mutex>;
	using Map = vt::map<Key, Value>;

	VT_INLINE ThreadSafeMap()
		: Map()
	{}

	VT_INLINE ThreadSafeMap(const ThreadSafeMap& other)
		: Map(other)
	{}

	VT_INLINE ThreadSafeMap(ThreadSafeMap&& other)
		: Map(std::move(other))
	{}

	VT_INLINE ThreadSafeMap& operator=(const ThreadSafeMap& other)
	{
		Map::operator=(other);
		return *this;
	}

	VT_INLINE ThreadSafeMap& operator=(ThreadSafeMap&& other)
	{
		Map::operator=(other);
		return *this;
	}

	VT_INLINE ~ThreadSafeMap() = default;

	VT_INLINE void clear()
	{
		WriteLock lock{ m_mutex };
		Map::clear();
	}

	VT_NODISCARD VT_INLINE constexpr bool contains(const Key& key) const
	{
		ReadLock lock{ m_mutex };
		return Map::contains(key);
	}

	VT_NODISCARD VT_INLINE Value& at(const Key& key)
	{
		ReadLock lock{ m_mutex };
		return Map::at(key);
	}

	VT_NODISCARD VT_INLINE const Value& at(const Key& key) const
	{
		ReadLock lock{ m_mutex };
		return Map::at(key);
	}

	VT_NODISCARD VT_INLINE Value& operator[](const Key& key)
	{
		WriteLock lock{ m_mutex };
		return Map::operator[](key);
	}

	VT_NODISCARD VT_INLINE const Value& operator[](const Key& key) const
	{
		ReadLock lock{ m_mutex };
		return Map::operator[](key);
	}

	VT_NODISCARD VT_INLINE Map::iterator begin() noexcept
	{
		WriteLock lock{ m_mutex };
		return Map::begin();
	}

	VT_NODISCARD VT_INLINE Map::iterator end() noexcept
	{
		WriteLock lock{ m_mutex };
		return Map::end();
	}

	VT_NODISCARD VT_INLINE Map::const_iterator begin() const noexcept
	{
		ReadLock lock{ m_mutex };
		return Map::begin();
	}

	VT_NODISCARD VT_INLINE Map::const_iterator end() const noexcept
	{
		ReadLock lock{ m_mutex };
		return Map::end();
	}

private:
	mutable std::shared_mutex m_mutex;
};
