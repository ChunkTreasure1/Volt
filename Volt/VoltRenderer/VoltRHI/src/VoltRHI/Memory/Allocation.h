#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	class Allocation : public RHIInterface
	{
	public:
		virtual ~Allocation() = default;

		template<typename T>
		constexpr T GetResourceHandle() const;

		template<typename T>
		constexpr T* Map();

		virtual void Unmap() = 0;

	protected:
		friend class Allocator;

		virtual void* GetResourceHandleInternal() const = 0;
		virtual void* MapInternal() = 0;

		Allocation() = default;
	};

	template<typename T>
	constexpr inline T Allocation::GetResourceHandle() const
	{
		return reinterpret_cast<T>(GetResourceHandleInternal());
	}

	template<typename T>
	constexpr inline T* Allocation::Map()
	{
		return reinterpret_cast<T*>(MapInternal());
	}
}
