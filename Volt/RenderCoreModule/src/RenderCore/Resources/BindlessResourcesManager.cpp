#include "rcpch.h"
#include "BindlessResourcesManager.h"

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

	void BindlessResourcesManager::UnregisterBuffer(ResourceHandle handle)
	{
		m_bindlessDescriptorTable->UnregisterBuffer(handle);
	}

	void BindlessResourcesManager::UnregisterImageView(ResourceHandle handle, RHI::ImageViewType viewType)
	{
		m_bindlessDescriptorTable->UnregisterImageView(handle, viewType);
	}

	void BindlessResourcesManager::UnregisterSamplerState(ResourceHandle handle)
	{
		m_bindlessDescriptorTable->UnregisterSamplerState(handle);
	}

	ResourceHandle BindlessResourcesManager::GetBufferHandle(WeakPtr<RHI::StorageBuffer> storageBuffer)
	{
		return m_bindlessDescriptorTable->GetBufferHandle(storageBuffer);
	}

	void BindlessResourcesManager::MarkBufferAsDirty(ResourceHandle handle)
	{
		m_bindlessDescriptorTable->MarkBufferAsDirty(handle);
	}

	void BindlessResourcesManager::MarkImageViewAsDirty(ResourceHandle handle, RHI::ImageViewType viewType)
	{
		m_bindlessDescriptorTable->MarkImageViewAsDirty(handle, viewType);
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
