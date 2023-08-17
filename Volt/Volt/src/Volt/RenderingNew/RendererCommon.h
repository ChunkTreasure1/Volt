#pragma once

#include <VoltRHI/Core/RHICommon.h>

namespace Volt
{
	struct IndirectBatchNew
	{
		RHI::IndirectCommand command{};
		uint32_t objectId{};
		uint32_t batchId{};
		uint32_t vertexBufferIndex{};
		uint32_t padding{};
	};
}
