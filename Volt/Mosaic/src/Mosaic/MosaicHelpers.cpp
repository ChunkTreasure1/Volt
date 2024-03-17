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

		if (typeInfo.columnCount > 1)
		{
			result += "x" + std::to_string(typeInfo.columnCount);
		}

		return result;
	}

	// We are trying to find the best matching type from two types.
	TypeInfo GetPromotedTypeInfo(const TypeInfo& A, const TypeInfo& B)
	{
		TypeInfo result;

		if (A.baseType == ValueBaseType::Float && B.baseType == ValueBaseType::Int || 
			A.baseType == ValueBaseType::Int && B.baseType == ValueBaseType::Float)
		{
			// Float can represent both integers and floating points and therefor is
			// higher ranked
			result.baseType = ValueBaseType::Float;
		}

		// Choose the highest number of elements
		result.vectorSize = std::max(A.vectorSize, B.vectorSize);
		result.columnCount = std::max(A.columnCount, B.columnCount);

		return result;
	}
}
