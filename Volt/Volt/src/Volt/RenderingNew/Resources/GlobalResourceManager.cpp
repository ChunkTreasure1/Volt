#include "vtpch.h"
#include "GlobalResourceManager.h"

#include "Volt/RenderingNew/Shader/ShaderMap.h"

#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/ImageView.h>
#include <VoltRHI/Shader/Shader.h>

namespace Volt
{
	static constexpr uint32_t TEXTURE1D_BINDING = 0;
	static constexpr uint32_t TEXTURE2D_BINDING = 1;
	static constexpr uint32_t TEXTURE3D_BINDING = 2;
	static constexpr uint32_t TEXTURECUBE_BINDING = 3;
	static constexpr uint32_t RWTEXTURE1D_BINDING = 4;
	static constexpr uint32_t RWTEXTURE2D_BINDING = 5;
	static constexpr uint32_t RWTEXTURE3D_BINDING = 6;

	static constexpr uint32_t BYTEADDRESSBUFFER_BINDING = 7;
	static constexpr uint32_t RWBYTEADDRESSBUFFER_BINDING = 8;

	void GlobalResourceManager::Initialize()
	{
		RHI::DescriptorTableCreateInfo info{};
		info.shader = RHI::Shader::Create("GlobalResourcesDescriptor", { "Engine/Shaders/Source/Utility/GlobalDescriptorsShader_cs.hlsl" }, true);
		info.count = 1;

		s_globalDescriptorTable = RHI::DescriptorTable::Create(info);
	}

	void GlobalResourceManager::Shutdown()
	{
		s_globalDescriptorTable = nullptr;
	}

	void GlobalResourceManager::Update()
	{
		// Buffers
		{
			auto& resources = GetResourceContainer<RHI::StorageBuffer>();
			std::scoped_lock lock{ resources.accessMutex };
			for (const auto& resource : resources.GetDirtyRange())

			{
				s_globalDescriptorTable->SetBufferView(resource->GetView(), 0, BYTEADDRESSBUFFER_BINDING, resources.GetResourceHandle(resource).Get());
				s_globalDescriptorTable->SetBufferView(resource->GetView(), 0, RWBYTEADDRESSBUFFER_BINDING, resources.GetResourceHandle(resource).Get());
			}

			resources.ClearDirty();
		}

		// Images
		{
			auto& resources = GetResourceContainer<RHI::ImageView>();
			std::scoped_lock lock{ resources.accessMutex };

			for (const auto& resource : resources.GetDirtyRange())
			{
				s_globalDescriptorTable->SetImageView(resource, 0, TEXTURE2D_BINDING, resources.GetResourceHandle(resource).Get());

				if (resource->GetImageUsage() == RHI::ImageUsage::Storage || resource->GetImageUsage() == RHI::ImageUsage::AttachmentStorage)
				{
					s_globalDescriptorTable->SetImageView(resource, 0, RWTEXTURE2D_BINDING, resources.GetResourceHandle(resource).Get());
				}
			}

			resources.ClearDirty();
		}

		// Samplers
		{
			auto& resources = GetResourceContainer<RHI::SamplerState>();
			std::scoped_lock lock{ resources.accessMutex };

			for (const auto& resource : resources.GetDirtyRange())
			{
				s_globalDescriptorTable->SetSamplerState(resource, 0, 10, resources.GetResourceHandle(resource).Get());
			}

			resources.ClearDirty();
		}

		s_globalDescriptorTable->Update();
	}
}
