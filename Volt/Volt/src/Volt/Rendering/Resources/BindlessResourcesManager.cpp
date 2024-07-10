#include "vtpch.h"
#include "BindlessResourcesManager.h"

#include "Volt/Core/Application.h"
#include "Volt/Core/Base.h"
#include "Volt/Rendering/RenderScene.h"

#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Buffers/BufferView.h>
#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/ImageView.h>
#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/RHIProxy.h>

namespace Volt
{
	BindlessResourcesManager::BindlessResourcesManager()
	{
		VT_CORE_ASSERT(!s_instance, "Instance should be null!");
		s_instance = this;

		m_bindlessDescriptorTable = RHI::BindlessDescriptorTable::Create();
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
