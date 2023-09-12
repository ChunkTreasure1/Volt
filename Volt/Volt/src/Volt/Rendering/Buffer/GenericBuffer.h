#pragma once

#include <vulkan/vulkan.h>

namespace Volt
{
	class GenericBuffer
	{
	public:
		GenericBuffer(VkBufferUsageFlags bufferUsage, uint32_t size, const void* data = nullptr);
		~GenericBuffer();

		const uint64_t GetDeviceAddress() const;

		static Ref<GenericBuffer> Create(VkBufferUsageFlags bufferUsage, uint32_t size, const void* data = nullptr);

	private:
		uint32_t mySize = 0;

		VkBuffer myBuffer = nullptr;
	};
}
