#pragma once

#include <CoreUtilities/UUID.h>
#include <CoreUtilities/FileIO/YAMLStreamWriter.h>
#include <CoreUtilities/FileIO/YAMLStreamReader.h>

#include <string>
#include <functional>

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

		bool showAttribute;

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

		std::function<void(YAMLStreamWriter& streamWriter, const Parameter& parameter)> serializationFunc;
		std::function<void(YAMLStreamReader& streamReader, Parameter& parameter)> deserializationFunc;
	};

	struct ResultInfo
	{
		std::string resultParamName;
		TypeInfo resultType;
	};
}
