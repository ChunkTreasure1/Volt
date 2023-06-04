#include "vtpch.h"
#include "CommandBufferCache.h"

#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	Ref<CommandBuffer> CommandBufferCache::GetOrCreateCommandBuffer(Ref<CommandBuffer> primaryCommandBuffer)
	{
		if (!myUnusedCommandBuffers.empty())
		{
			Ref<CommandBuffer> cmdBuffer = myUnusedCommandBuffers.back();
			myUnusedCommandBuffers.pop_back();

			myUsedCommandBuffers.emplace_back(cmdBuffer);

			return cmdBuffer;
		}

		Ref<CommandBuffer> newCmdBuffer = CommandBuffer::Create(Renderer::GetFramesInFlightCount(), primaryCommandBuffer);
		myUsedCommandBuffers.emplace_back(newCmdBuffer);

		return newCmdBuffer;
	}
	
	void CommandBufferCache::Reset()
	{
		for (const auto& usedCmdBuffer : myUsedCommandBuffers)
		{
			myUnusedCommandBuffers.emplace_back(usedCmdBuffer);
		}

		myUsedCommandBuffers.clear();
	}
}
