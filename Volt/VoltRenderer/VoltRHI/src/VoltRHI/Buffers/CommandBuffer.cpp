#include "rhipch.h"
#include "CommandBuffer.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltMock/Buffers/MockCommandBuffer.h>

namespace Volt
{
	Ref<CommandBuffer> CommandBuffer::Create(const uint32_t count, QueueType queueType)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::Vulkan:
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Mock: return CreateRefRHI<MockCommandBuffer>(count, queueType); break;
		}

		return nullptr;
	}
	
	CommandBuffer::CommandBuffer(QueueType queueType)
		: m_queueType(queueType)
	{
	}
}
