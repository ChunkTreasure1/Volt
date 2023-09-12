#pragma once

#include <vulkan/vulkan.h>

namespace Volt
{
	class UniformBuffer
	{
	public:
		UniformBuffer(uint32_t size, const void* data);
		UniformBuffer(uint32_t sizePerElement, uint32_t elementCount);
		~UniformBuffer();

		inline const VkBuffer GetHandle() const { return myBuffer; }
		inline const uint32_t GetSize() const { return mySize; }
		inline const uint32_t GetTotalSize() const { return myTotalSize; }

		inline const bool IsDynamic() const { return myIsDynamic; }

		void SetData(const void* data, uint32_t size);

		template<typename T>
		T* Map();

		void Unmap();

		static Ref<UniformBuffer> Create(uint32_t size, const void* data = nullptr);
		static Ref<UniformBuffer> Create(uint32_t sizePerElement, uint32_t elementCount);

	private:
		uint32_t mySize = 0;
		uint32_t myTotalSize = 0;
		bool myIsDynamic = false;

		VkBuffer myBuffer = nullptr;
	};

	template<typename T>
	inline T* UniformBuffer::Map()
	{
	}
}
