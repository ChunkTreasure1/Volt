#pragma once

#include "VoltRHI/Core/Core.h"
namespace Volt
{
	class RHI_API RHIInterface : public std::enable_shared_from_this<RHIInterface>
	{
	public:
		virtual ~RHIInterface() = default;
		VT_DELETE_COMMON_OPERATORS(RHIInterface);

		template<typename T>
		constexpr T GetHandle()
		{
			return reinterpret_cast<T>(GetHandleImpl());
		}

		template<typename T>
		constexpr Ref<T> As()
		{
			return std::reinterpret_pointer_cast<T>(shared_from_this());
		}

	protected:
		RHIInterface() = default;

		virtual void* GetHandleImpl() = 0;
	};
}
