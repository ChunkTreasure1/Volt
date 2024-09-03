#pragma once

#include <RHIModule/Descriptors/ResourceHandle.h>

namespace Volt
{
	struct DrawContext
	{
		ResourceHandle instanceOffsetToObjectIDBuffer;

		ResourceHandle drawIndexToObjectId;
		ResourceHandle drawIndexToMeshletId;
	};
}

