#include "rcpch.h"
#include "RenderCore/Resources/BindlessResourcesManager.h"

#include <RHIModule/Buffers/StorageBuffer.h>
#include <RHIModule/Buffers/BufferView.h>
#include <RHIModule/Images/ImageView.h>
#include <RHIModule/Shader/Shader.h>
#include <RHIModule/Images/SamplerState.h>
#include <RHIModule/RHIProxy.h>

namespace Volt
{
	BindlessResourcesManager::BindlessResourcesManager()
	{
		VT_ASSERT_MSG(!s_instance, "Instance should be null!");
		s_instance = this;

		constexpr uint32_t framesInFlight = 3;
		m_bindlessDescriptorTable = RHI::BindlessDescriptorTable::Create(framesInFlight);
	}

	BindlessResourcesManager::~BindlessResourcesManager()
	{
		m_bindlessDescriptorTable = nullptr;
		s_instance = nullptr;
	}

	ResourceHandle BindlessResourcesManager::RegisterBuffer(WeakPtr<RHI::StorageBuffer> storageBuffer)
	{
		return m_bindlessDescriptorTable->RegisterBuffer(storageBuffer);
	}

	ResourceHandle BindlessResourcesManager::RegisterImageView(WeakPtr<RHI::ImageView> imageView)
	{
		return m_bindlessDescriptorTable->RegisterImageView(imageView);
	}

	ResourceHandle BindlessResourcesManager::RegisterSamplerState(WeakPtr<RHI::SamplerState> samplerState)
	{
		return m_bindlessDescriptorTable->RegisterSamplerState(samplerState);
	}

	void BindlessResourcesManager::UnregisterResource(ResourceHandle handle)
	{
		m_bindlessDescriptorTable->UnregisterResource(handle);
	}

	void BindlessResourcesManager::MarkResourceAsDirty(ResourceHandle handle)
	{
		m_bindlessDescriptorTable->MarkResourceAsDirty(handle);
	}

	void BindlessResourcesManager::UnregisterSamplerState(ResourceHandle handle)
	{
		m_bindlessDescriptorTable->UnregisterSamplerState(handle);
	}

	void BindlessResourcesManager::MarkSamplerStateAsDirty(ResourceHandle handle)
	{
		m_bindlessDescriptorTable->MarkSamplerStateAsDirty(handle);
	}

	void BindlessResourcesManager::Update()
	{
		m_bindlessDescriptorTable->Update();
	}

	void BindlessResourcesManager::PrepareForRender()
	{
		m_bindlessDescriptorTable->PrepareForRender();
	}
}
