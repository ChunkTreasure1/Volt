#pragma once

#include "Volt/Asset/Asset.h"

#include <vulkan/vulkan.h>
#include <map>

namespace Volt
{
	class Image2D;
	class GlobalDescriptorSet;

	class TextureTable
	{
	public:
		TextureTable();

		void RemoveTexture(Ref<Image2D> texture);

		const uint32_t AddTexture(Ref<Image2D> texture);
		const uint32_t GetBindingFromTexture(Ref<Image2D> texture);


		void UpdateDescriptors(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t index);
		const std::vector<VkWriteDescriptorSet> GetWriteDescriptors(VkDescriptorSet targetSet, uint32_t binding);

		inline static Ref<TextureTable> Create() { return CreateRef<TextureTable>(); }

		inline static constexpr uint32_t TEXTURE2D_COUNT = 1024;
		inline static constexpr uint32_t TEXTURECUBE_COUNT = 1024;
		inline static constexpr uint32_t TEXTURE3D_COUNT = 1024;

	private:
		void SetDirty(bool state);

		std::vector<bool> myIsDirty;
		uint32_t myNextBinding = 1;

		std::vector<uint32_t> myAvailableBindings;
		std::map<Ref<Image2D>, uint32_t> myTextureBindingMap;
	};
}
