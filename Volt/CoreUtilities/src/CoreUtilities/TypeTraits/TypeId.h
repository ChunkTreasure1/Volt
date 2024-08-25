#pragma once

#include <string_view>

namespace TypeTraits
{
	typedef size_t TypeID;

	static constexpr size_t GetTypeNameHash(std::string_view str)
	{
		constexpr size_t FNVOffsetBasis = 14695981039346656037ULL;
		constexpr size_t FNVPrime = 1099511628211ULL;

		size_t val = FNVOffsetBasis;

		for (char c : str)
		{
			val ^= static_cast<size_t>(c);
			val *= FNVPrime;
		}

		return val;
	}

	template<typename T>
	constexpr TypeID TypeId()
	{
		return GetTypeNameHash(__FUNCSIG__);
	}
}
