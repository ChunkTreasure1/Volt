#pragma once

#include "TypeId.h"

namespace TypeTraits
{
	struct TypeIndex
	{
		explicit constexpr TypeIndex(TypeID inId) noexcept
			: id(inId)
		{ }

		constexpr TypeIndex(const TypeIndex& other) noexcept
			: id(other.id)
		{
		}

		template<typename T>
		constexpr static TypeIndex FromType()
		{
			return TypeIndex(TypeId<T>());
		}

		constexpr bool operator==(const TypeIndex& other) const
		{
			return id == other.id;
		}

		constexpr bool operator!=(const TypeIndex& other) const
		{
			return id == other.id;
		}

		constexpr TypeIndex& operator=(const TypeIndex& other)
		{
			id = other.id;
			return *this;
		}

		constexpr TypeIndex& operator=(const TypeID& other)
		{
			id = other;
			return *this;
		}

		TypeID id;
	};
}

namespace std
{
	template<>
	struct hash<TypeTraits::TypeIndex>
	{
		size_t operator()(const TypeTraits::TypeIndex& val) const noexcept
		{
			return val.id;
		}
	};
}
