#include "vtpch.h"
#include "TextureTable.h"

#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"
#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/GlobalDescriptorSet.h"
#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	TextureTable::TextureTable()
	{
		myIsDirty = std::vector<bool>(Renderer::GetFramesInFlightCount(), false);
	}

	void TextureTable::RemoveTexture(Ref<Image2D> texture)
	{
		if (!myTextureBindingMap.contains(texture))
		{
			return;
		}

		myAvailableBindings.emplace_back(myTextureBindingMap.at(texture));
		myTextureBindingMap.erase(texture);
	
		SetDirty(true);
	}

	const uint32_t TextureTable::AddTexture(Ref<Image2D> texture)
	{
		uint32_t binding = 0;

		if (!myAvailableBindings.empty())
		{
			binding = myAvailableBindings.back();
			myAvailableBindings.pop_back();
		}
		else
		{
			binding = myNextBinding++;
		}

		myTextureBindingMap.emplace(texture, binding);

		SetDirty(true);

		return binding;
	}

	const uint32_t TextureTable::GetBindingFromTexture(Ref<Image2D> texture)
	{
		if (!myTextureBindingMap.contains(texture))
		{
			return 0; // Zero is invalid
		}

		return myTextureBindingMap.at(texture);
	}
	
	void TextureTable::UpdateDescriptors(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t index)
	{
		VT_PROFILE_FUNCTION();

		if (!myIsDirty.at(index))
		{
			return;
		}

		myIsDirty.at(index) = false;

		auto descriptorSet = globalDescriptorSet->GetOrAllocateDescriptorSet(index);
		const auto writeDescriptors = GetWriteDescriptors(descriptorSet, Bindings::TEXTURE2DTABLE);

		if (writeDescriptors.empty())
		{
			return;
		}

		auto device = GraphicsContextVolt::GetDevice();
		vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}

	const std::vector<VkWriteDescriptorSet> TextureTable::GetWriteDescriptors(VkDescriptorSet targetSet, uint32_t binding)
	{
		VT_PROFILE_FUNCTION();

		std::vector<VkWriteDescriptorSet> writeDescriptors{ myTextureBindingMap.size() };

		for (uint32_t currentWriteDescriptor = 0; const auto& [texture, index] : myTextureBindingMap)
		{
			auto& writeDescriptor = writeDescriptors.at(currentWriteDescriptor);
			writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptor.descriptorCount = 1;
			writeDescriptor.dstArrayElement = index;
			writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			writeDescriptor.dstSet = targetSet;
			writeDescriptor.dstBinding = binding;
			writeDescriptor.pImageInfo = &texture->GetDescriptorInfo();

			currentWriteDescriptor++;
		}

		return writeDescriptors;
	}
	
	void TextureTable::SetDirty(bool state)
	{
		for (const auto& val : myIsDirty)
		{
			val = state;
		}
	}
}
