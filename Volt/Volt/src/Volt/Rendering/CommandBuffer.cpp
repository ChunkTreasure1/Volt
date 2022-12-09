#include "vtpch.h"
#include "CommandBuffer.h"

namespace Volt
{
	void CommandBuffer::Clear()
	{
		myCommands.clear();
		myRenderCommands.clear();
		myCurrentRenderCommands.clear();
	}

	void CommandBuffer::Execute()
	{
		for (const auto& cmd : myCommands)
		{
			cmd();
		}
	}

	void CommandBuffer::Submit(const std::function<void()> && func)
	{
		myCommands.emplace_back(func);
	}

	void CommandBuffer::Submit(const RenderCommand& cmd)
	{
		myRenderCommands.emplace_back(cmd);
	}
}