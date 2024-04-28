#pragma once

#include <cstdint>
#include <xhash>
#include <string_view>

#ifndef VT_DIST
	#define STRING_HASH_CONSTEXPR
#else
	#define STRING_HASH_CONSTEXPR constexpr
#endif

struct StringHash
{
	STRING_HASH_CONSTEXPR StringHash()
		: hash(0)
	{ }

	STRING_HASH_CONSTEXPR StringHash(const StringHash& rhs)
		: hash(rhs.hash)
#ifndef VT_DIST
		, string(rhs.string)
#endif
	{}

	STRING_HASH_CONSTEXPR StringHash(const size_t& inHash)
		: hash(inHash)
	{}

#ifndef VT_DIST
	StringHash(const size_t& inHash, const std::string& inString)
		: hash(inHash), string(inString)
	{ }
#endif

	STRING_HASH_CONSTEXPR static StringHash Construct(const size_t& inHash)
	{
		return StringHash(inHash);
	}

	STRING_HASH_CONSTEXPR static StringHash Construct(std::string_view str)
	{
		constexpr size_t FNVOffsetBasis = 14695981039346656037ULL;
		constexpr size_t FNVPrime = 1099511628211ULL;

		size_t val = FNVOffsetBasis;

		for (char c : str)
		{
			val ^= static_cast<size_t>(c);
			val *= FNVPrime;
		}

#ifndef VT_DIST
		return StringHash(val, std::string(str));
#else
		return StringHash(val);
#endif
	}

	constexpr bool operator==(const StringHash& rhs) const { return hash == rhs.hash; }
	constexpr bool operator!=(const StringHash& rhs) const { return hash != rhs.hash; }
	constexpr bool operator<(const StringHash& rhs) const { return hash < rhs.hash; }
	constexpr bool operator>(const StringHash& rhs) const { return hash > rhs.hash; }
	constexpr bool operator<=(const StringHash& rhs) const { return hash < rhs.hash; }
	constexpr bool operator>=(const StringHash& rhs) const { return hash > rhs.hash; }

	size_t hash;

#ifndef VT_DIST
	std::string string;
#endif
};

STRING_HASH_CONSTEXPR inline StringHash operator"" _sh(const char* input, size_t)
{
	return StringHash::Construct(std::string_view(input));
}

namespace std
{
	template<>
	struct hash<StringHash>
	{
		size_t operator()(const StringHash& hash) const
		{
			return hash.hash;
		}
	};
}
