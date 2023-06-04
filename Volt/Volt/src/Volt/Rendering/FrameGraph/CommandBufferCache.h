#pragma once

namespace Volt
{
	class CommandBuffer;
	class CommandBufferCache
	{
	public:
		Ref<CommandBuffer> GetOrCreateCommandBuffer(Ref<CommandBuffer> primaryCommandBuffer);
		void Reset();

		inline const std::vector<Ref<CommandBuffer>>& GetUsedCommandBuffers() const { return myUsedCommandBuffers; }

	private:
		std::vector<Ref<CommandBuffer>> myUsedCommandBuffers;
		std::vector<Ref<CommandBuffer>> myUnusedCommandBuffers;
	};
}
