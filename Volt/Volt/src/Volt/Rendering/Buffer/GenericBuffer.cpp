#include "vtpch.h"
#include "GenericBuffer.h"

#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	GenericBuffer::GenericBuffer(VkBufferUsageFlags bufferUsage, uint32_t size, const void* data)
		: mySize(size)
	{
		const VkDeviceSize bufferSize = (VkDeviceSize)size;

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = bufferSize;
		info.usage = bufferUsage;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


	}

	GenericBuffer::~GenericBuffer()
	{
		if (myBuffer == VK_NULL_HANDLE)
		{
			return;
		}

	}

	const uint64_t GenericBuffer::GetDeviceAddress() const
	{
		VkBufferDeviceAddressInfo bufferAddr{};
		bufferAddr.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferAddr.pNext = nullptr;
		bufferAddr.buffer = myBuffer;

		//auto device = GraphicsContextVolt::GetDevice();

		return /*vkGetBufferDeviceAddress(device->GetHandle(), &bufferAddr)*/ 0;
	}

	Ref<GenericBuffer> GenericBuffer::Create(VkBufferUsageFlags bufferUsage, uint32_t size, const void* data)
	{
		return CreateRef<GenericBuffer>(bufferUsage, size, data);
	}
}
