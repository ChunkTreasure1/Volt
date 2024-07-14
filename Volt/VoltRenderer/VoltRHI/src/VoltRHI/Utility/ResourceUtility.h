#pragma once

#include "VoltRHI/Core/RHICommon.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

namespace Volt::RHI::ResourceUtility
{
	inline void InitializeBarrierSrcFromCurrentState(ImageBarrier& barrier, WeakPtr<RHIResource> resource)
	{
		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(resource);
		barrier.srcStage = currentState.stage;
		barrier.srcAccess = currentState.access;
		barrier.srcLayout = currentState.layout;
	}

	inline void InitializeBarrierSrcFromCurrentState(BufferBarrier& barrier, WeakPtr<RHIResource> resource)
	{
		const auto& currentState = GraphicsContext::GetResourceStateTracker()->GetCurrentResourceState(resource);
		barrier.srcAccess = currentState.access;
		barrier.srcStage = currentState.stage;
	}
}
