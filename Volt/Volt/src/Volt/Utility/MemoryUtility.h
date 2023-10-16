#pragma once

#include <cstdint>

namespace Volt::Utility
{
	// Get offset of a member variable
	template<typename Type, typename MemberType> 
	inline ptrdiff_t GetMemberOffset(MemberType Type::* pMember)
	{
		return reinterpret_cast<uint8_t*>(&(reinterpret_cast<Type*>(1)->*pMember)) - reinterpret_cast<uint8_t*>(1);
	}

	// Get offset of a base structure / class
	template<typename Type, typename BaseType> 
	inline ptrdiff_t GetBaseOffset()
	{
		return reinterpret_cast<uint8_t*>(static_cast<BaseType*>(reinterpret_cast<Type*>(1))) - reinterpret_cast<uint8_t*>(1);
	}
}
