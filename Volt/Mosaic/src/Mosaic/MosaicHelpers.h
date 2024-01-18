#pragma once

#include "Mosaic/Parameter.h"

namespace Mosaic::Helpers
{
	extern std::string GetTypeNameFromTypeInfo(const TypeInfo& typeInfo);
	extern TypeInfo GetPromotedTypeInfo(const TypeInfo& A, const TypeInfo& B);
}
