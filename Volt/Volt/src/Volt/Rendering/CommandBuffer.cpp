#include "vtpch.h"
#include "CommandBuffer.h"

#include <d3d11.h>

namespace Volt
{
	void CommandBuffer::Clear()
	{
		myCommands.clear();
	}

	void CommandBuffer::Execute()
	{
		for (const auto& cmd : myCommands)
		{
			cmd();
		}
	}

	void CommandBuffer::Submit(const std::function<void()>&& func)
	{
		myCommands.emplace_back(func);
	}

	ID3D11CommandList*& CommandBuffer::GetAndReleaseCommandList()
	{
		if (myCommandList)
		{
			myCommandList->Release();
		}
		
		return myCommandList;
	}
}