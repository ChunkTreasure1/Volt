#pragma once

#include "VoltRHI/Core/Core.h"

namespace Volt::RHI
{
	class VTRHI_API RHIInterface
	{
	public:
		virtual ~RHIInterface() = default;
		VT_DELETE_COMMON_OPERATORS(RHIInterface);

		template<typename T>
		constexpr T GetHandle() const
		{
			return reinterpret_cast<T>(GetHandleImpl());
		}

		template<typename T>
		constexpr T* As()
		{
			return reinterpret_cast<T*>(this);
		}

		template<typename T>
		constexpr T& AsRef()
		{
			return *reinterpret_cast<T*>(this);
		}

	protected:
		RHIInterface() = default;

		virtual void* GetHandleImpl() const = 0;
	};
}
