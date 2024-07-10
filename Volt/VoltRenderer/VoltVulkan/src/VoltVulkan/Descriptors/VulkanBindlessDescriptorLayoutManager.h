#pragma once

struct VkDescriptorSetLayout_T;
struct VkDescriptorSet_T;

namespace Volt::RHI
{
	class VulkanBindlessDescriptorLayoutManager
	{
	public:
		inline static constexpr uint32_t TEXTURE1D_BINDING = 0;
		inline static constexpr uint32_t TEXTURE2D_BINDING = 1;
		inline static constexpr uint32_t TEXTURE3D_BINDING = 2;
		inline static constexpr uint32_t TEXTURECUBE_BINDING = 3;
		inline static constexpr uint32_t RWTEXTURE1D_BINDING = 4;
		inline static constexpr uint32_t RWTEXTURE2D_BINDING = 5;
		inline static constexpr uint32_t RWTEXTURE3D_BINDING = 6;
		inline static constexpr uint32_t BYTEADDRESSBUFFER_BINDING = 7;
		inline static constexpr uint32_t RWBYTEADDRESSBUFFER_BINDING = 8;
		inline static constexpr uint32_t UNIFORMBUFFER_BINDING = 9;
		inline static constexpr uint32_t SAMPLERSTATE_BINDING = 10;
		inline static constexpr uint32_t RWTEXTURE2DARRAY_BINDING = 11;
		inline static constexpr uint32_t TEXTURE2DARRAY_BINDING = 12;

		static void CreateGlobalDescriptorLayout();
		static void DestroyGlobalDescriptorLayout();

		static VkDescriptorSetLayout_T* GetGlobalDescriptorSetLayout();

	private:

		VulkanBindlessDescriptorLayoutManager() = delete;
	};
}
