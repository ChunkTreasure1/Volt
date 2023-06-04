#include "vtpch.h"
#include "GenericBuffer.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Core/Graphics/GraphicsDevice.h"

#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	GenericBuffer::GenericBuffer(VkBufferUsageFlags bufferUsage, uint32_t size, const void* data)
		: mySize(size)
	{
		const VkDeviceSize bufferSize = (VkDeviceSize)size;
		VulkanAllocator allocator{ "GenericBuffer - Create" };

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = bufferUsage;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		myAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU , myBuffer);

		void* bufferData = allocator.MapMemory<void*>(myAllocation);
		memcpy_s(bufferData, mySize, data, size);
		allocator.UnmapMemory(myAllocation);
	}

	GenericBuffer::~GenericBuffer()
	{
		if (myBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		Renderer::SubmitResourceChange([buffer = myBuffer, allocation = myAllocation]()
		{
			VulkanAllocator allocator{};
			allocator.DestroyBuffer(buffer, allocation);
		});
	}

	const uint64_t GenericBuffer::GetDeviceAddress() const
	{
		VkBufferDeviceAddressInfo bufferAddr{};
		bufferAddr.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferAddr.pNext = nullptr;
		bufferAddr.buffer = myBuffer;

		auto device = GraphicsContext::GetDevice();

		return vkGetBufferDeviceAddress(device->GetHandle(), &bufferAddr);
	}

	Ref<GenericBuffer> GenericBuffer::Create(VkBufferUsageFlags bufferUsage, uint32_t size, const void* data)
	{
		return CreateRef<GenericBuffer>(bufferUsage, size, data);
	}
}
