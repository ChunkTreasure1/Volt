#include "vtpch.h"
#include "CommandBuffer.h"

#include <d3d11.h>

namespace Volt
{
	CommandBuffer::CommandBuffer()
	{
		constexpr uint32_t totalSize = 10 * 1024 * 1024;
		myCommandBufferAllocation = new uint8_t[totalSize];
		myCommandBufferPtr = myCommandBufferAllocation;
		
		memset(myCommandBufferAllocation, 0, totalSize);
	}

	CommandBuffer::~CommandBuffer()
	{
		delete[] myCommandBufferAllocation;
		myCommandBufferPtr = nullptr;
	}

	void CommandBuffer::Clear()
	{
		myCommandBufferPtr = myCommandBufferAllocation;
		myCommandCount = 0;
	}

	void CommandBuffer::Execute()
	{
		uint8_t* buffer = myCommandBufferAllocation;
		for (uint32_t i = 0; i < myCommandCount; i++)
		{
			CommandFn function = *(CommandFn*)buffer;
			buffer += sizeof(CommandFn);
			
			const uint32_t size = *(uint32_t*)buffer;
			buffer += sizeof(uint32_t);
			function(buffer);
			buffer += size;
		}
	}

	ID3D11CommandList*& CommandBuffer::GetAndReleaseCommandList()
	{
		if (myCommandList)
		{
			myCommandList->Release();
		}
		
		return myCommandList;
	}
	
	void* CommandBuffer::AllocateCommand(CommandFn func, uint32_t size)
	{
		*(CommandFn*)myCommandBufferPtr = func;
		myCommandBufferPtr += sizeof(CommandFn);

		*(uint32_t*)myCommandBufferPtr = size;
		myCommandBufferPtr += sizeof(size);

		void* memory = myCommandBufferPtr;
		
		myCommandBufferPtr += size;
		myCommandCount++;

		return memory;
	}
}