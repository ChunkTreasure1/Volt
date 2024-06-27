#pragma once

#include <CoreUtilities/Core.h>

namespace Volt::RHI
{
	enum class D3D12ViewType
	{
		None = 0,
		RTV = 1 << 0,
		DSV = 1 << 1,
		SRV = 1 << 2,
		UAV = 1 << 3,
		CBV = 1 << 4,
		Sampler = 1 << 5
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(D3D12ViewType);
}
