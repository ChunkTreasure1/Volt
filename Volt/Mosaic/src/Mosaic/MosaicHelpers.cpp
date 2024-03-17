#include "mcpch.h"
#include "MosaicHelpers.h"

namespace Mosaic::Helpers
{
	std::string GetTypeNameFromTypeInfo(const TypeInfo& typeInfo)
	{
		std::string result;

		switch (typeInfo.baseType)
		{
			case ValueBaseType::Float: result = "float"; break;
			case ValueBaseType::Int: result = "int"; break;
		}

		if (typeInfo.vectorSize == 1)
		{
			return result;
		}

		result += std::to_string(typeInfo.vectorSize);

		return result;
	}
}
