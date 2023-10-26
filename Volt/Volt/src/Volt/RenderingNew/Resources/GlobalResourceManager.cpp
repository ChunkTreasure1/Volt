#include "vtpch.h"
#include "GlobalResourceManager.h"

#include "Volt/RenderingNew/Shader/ShaderMap.h"

#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Shader/Shader.h>

namespace Volt
{
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
				s_globalDescriptorTable->SetBufferView(resource->GetView(), 0, 7, resources.GetResourceHandle(resource));
				s_globalDescriptorTable->SetBufferView(resource->GetView(), 0, 8, resources.GetResourceHandle(resource));
			}

			resources.ClearDirty();
		}

		// Images
		{
			auto& resources = GetResourceContainer<RHI::Image2D>();
			std::scoped_lock lock{ resources.accessMutex };

			for (const auto& resource : resources.GetDirtyRange())
			{
				s_globalDescriptorTable->SetImageView(resource->GetView(), 0, 1, resources.GetResourceHandle(resource));
				s_globalDescriptorTable->SetImageView(resource->GetView(), 0, 5, resources.GetResourceHandle(resource));
			}

			resources.ClearDirty();
		}

		// Samplers
		{
			auto& resources = GetResourceContainer<RHI::SamplerState>();
			std::scoped_lock lock{ resources.accessMutex };

			for (const auto& resource : resources.GetDirtyRange())
			{
				s_globalDescriptorTable->SetSamplerState(resource, 0, 10, resources.GetResourceHandle(resource));
			}

			resources.ClearDirty();
		}

		s_globalDescriptorTable->Update();
	}
}
