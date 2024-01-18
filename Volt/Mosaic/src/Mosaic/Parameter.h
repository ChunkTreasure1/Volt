#pragma once

#include <string>

#include <CoreUtilities/UUID.h>

namespace Mosaic
{
	enum class ValueBaseType : uint8_t
	{
		Float,
		Int,
		Bool,

		Dynamic
	};

	struct TypeInfo
	{
		ValueBaseType baseType = ValueBaseType::Float;
		uint32_t vectorSize = 1;
		uint32_t columnCount = 1;
	};

	enum class ParameterDirection
	{
		Input,
		Output
	};

	struct Parameter
	{
		std::string name;
		TypeInfo typeInfo;

		ParameterDirection direction;
		uint8_t dataArray[64];

		UUID64 id{};
		uint32_t index = 0;

		template<typename T>
		inline T& Get()
		{
			return *reinterpret_cast<T*>(dataArray);
		}

		template<typename T>
		inline const T& Get() const
		{
			return *reinterpret_cast<const T*>(dataArray);
		}
	};

	struct ResultInfo
	{
		std::string resultParamName;
		TypeInfo resultType;
	};
}
