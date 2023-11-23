#pragma once

#include <string>

namespace Mosaic
{
	enum class ValueBaseType : uint8_t
	{
		Float,
		Int,

		Dynamic
	};

	struct TypeInfo
	{
		ValueBaseType baseType = ValueBaseType::Float;
		uint32_t vectorSize = 1;
	};

	struct InputParameter
	{
		std::string name;
	};

	struct ReturnValue
	{
		std::string name;
	};
}
