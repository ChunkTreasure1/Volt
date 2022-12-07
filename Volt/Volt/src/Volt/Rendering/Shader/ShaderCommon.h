#pragma once

#include "Volt/Core/Base.h"

namespace Volt
{
	enum class ShaderStage : uint32_t
	{
		None = 0,
		Vertex = BIT(0),
		Pixel = BIT(1),
		Hull = BIT(2),
		Domain = BIT(3),
		Geometry = BIT(4),
		Compute = BIT(5)
	};

	inline ShaderStage operator|(ShaderStage aLhs, ShaderStage aRhs)
	{
		return (ShaderStage)((std::underlying_type<ShaderStage>::type)aLhs | (std::underlying_type<ShaderStage>::type)aRhs);
	}

	inline ShaderStage operator&(ShaderStage aLhs, ShaderStage aRhs)
	{
		return (ShaderStage)((std::underlying_type<ShaderStage>::type)aLhs & (std::underlying_type<ShaderStage>::type)aRhs);
	}
}