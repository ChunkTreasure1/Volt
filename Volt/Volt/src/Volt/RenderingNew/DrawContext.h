#pragma once

#include "Volt/RenderingNew/Resources/ResourceHandle.h"

namespace Volt
{
	struct DrawContext
	{
		ResourceHandle instanceOffsetToObjectIDBuffer;

		ResourceHandle drawIndexToObjectId;
		ResourceHandle drawIndexToMeshletId;
	};
}

