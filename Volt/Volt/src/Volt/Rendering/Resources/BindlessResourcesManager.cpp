#include "vtpch.h"
#include "BindlessResourcesManager.h"

#include "Volt/Core/Application.h"

#include <VoltRHI/Buffers/StorageBuffer.h>
#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/ImageView.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/RHIProxy.h>

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
	static constexpr uint32_t SAMPLERSTATES_BINDING = 10;

	static constexpr uint32_t RWTEXTURE2DARRAY_BINDING = 11;
	static constexpr uint32_t TEXTURE2DARRAY_BINDING = 12;

	BindlessResourcesManager::BindlessResourcesManager()
	{
		VT_CORE_ASSERT(!s_instance, "Instance should be null!");
		s_instance = this;

		RHI::DescriptorTableCreateInfo tableInfo{};
	
		RHI::ShaderSpecification shaderSpec{};
		shaderSpec.name = "GlobalResourcesDescriptor";
		shaderSpec.sourceFiles = { "Engine/Shaders/Source/Utility/GlobalDescriptorsShader_cs.hlsl" };
		shaderSpec.forceCompile = true;

		tableInfo.shader = RHI::Shader::Create(shaderSpec);
		tableInfo.isGlobal = true;

		m_bindlessDescriptorTable = RHI::DescriptorTable::Create(tableInfo);
	}

	BindlessResourcesManager::~BindlessResourcesManager()
	{
		m_bindlessDescriptorTable = nullptr;
		s_instance = nullptr;
	}

	ResourceHandle BindlessResourcesManager::RegisterBuffer(Weak<RHI::StorageBuffer> storageBuffer)
	{
		return m_bufferRegistry.RegisterResource(storageBuffer.GetSharedPtr());
	}

	ResourceHandle BindlessResourcesManager::RegisterImageView(Weak<RHI::ImageView> imageView)
	{
		const auto viewType = imageView->GetViewType();

		if (viewType == RHI::ImageViewType::View2D)
		{
			return m_image2DRegistry.RegisterResource(imageView.GetSharedPtr(), imageView->GetImageUsage());
		}
		else if (viewType == RHI::ImageViewType::View2DArray)
		{
			return m_image2DArrayRegistry.RegisterResource(imageView.GetSharedPtr(), imageView->GetImageUsage());
		}
		else if (viewType == RHI::ImageViewType::ViewCube)
		{
			return m_imageCubeRegistry.RegisterResource(imageView.GetSharedPtr(), imageView->GetImageUsage());
		}

		VT_CORE_ASSERT(false, "Resource type not implemented!");
		return Resource::Invalid;
	}

	ResourceHandle BindlessResourcesManager::RegisterSamplerState(Weak<RHI::SamplerState> samplerState)
	{
		return m_samplerRegistry.RegisterResource(samplerState);
	}

	void BindlessResourcesManager::UnregisterBuffer(ResourceHandle handle)
	{
		m_bufferRegistry.UnregisterResource(handle);
	}

	void BindlessResourcesManager::UnregisterImageView(ResourceHandle handle, RHI::ImageViewType viewType)
	{
		VT_CORE_ASSERT(viewType == RHI::ImageViewType::View2D || viewType == RHI::ImageViewType::View2DArray || viewType == RHI::ImageViewType::ViewCube, "View type is not implemented!");

		if (viewType == RHI::ImageViewType::View2D)
		{
			m_image2DRegistry.UnregisterResource(handle);
		}
		else if (viewType == RHI::ImageViewType::View2DArray)
		{
			m_image2DArrayRegistry.UnregisterResource(handle);
		}
		else if (viewType == RHI::ImageViewType::ViewCube)
		{
			m_imageCubeRegistry.UnregisterResource(handle);
		}
	}

	void BindlessResourcesManager::UnregisterSamplerState(ResourceHandle handle)
	{
		m_samplerRegistry.UnregisterResource(handle);
	}

	ResourceHandle BindlessResourcesManager::GetBufferHandle(Weak<RHI::StorageBuffer> storageBuffer)
	{
		return m_bufferRegistry.GetResourceHandle(storageBuffer);
	}

	void BindlessResourcesManager::MarkBufferAsDirty(ResourceHandle handle)
	{
		m_bufferRegistry.MarkAsDirty(handle);
	}

	void BindlessResourcesManager::MarkImageViewAsDirty(ResourceHandle handle, RHI::ImageViewType viewType)
	{
		VT_CORE_ASSERT(viewType == RHI::ImageViewType::View2D || viewType == RHI::ImageViewType::View2DArray || viewType == RHI::ImageViewType::ViewCube, "View type is not implemented!");

		if (viewType == RHI::ImageViewType::View2D)
		{
			m_image2DRegistry.MarkAsDirty(handle);
		}
		else if (viewType == RHI::ImageViewType::View2DArray)
		{
			m_image2DArrayRegistry.MarkAsDirty(handle);
		}
		else if (viewType == RHI::ImageViewType::ViewCube)
		{
			m_imageCubeRegistry.MarkAsDirty(handle);
		}
	}

	void BindlessResourcesManager::MarkSamplerStateAsDirty(ResourceHandle handle)
	{
		m_samplerRegistry.MarkAsDirty(handle);
	}

	void BindlessResourcesManager::PrintResources()
	{
		VT_CORE_TRACE("Frame Index: {0}", Application::GetFrameIndex());
		VT_CORE_TRACE("---------- Registered Bindless Buffers ----------");
		{
			std::scoped_lock lock{ m_bufferRegistry.m_mutex };
			for (const auto& resource : m_bufferRegistry.m_resources)
			{
				if (resource.referenceCount == 0)
				{
					continue;
				}

				VT_CORE_TRACE("Buffer: {0}", resource.handle.Get());
			}
		}

		VT_CORE_TRACE("---------- Registered Bindless Image Views ----------");

		{
			std::scoped_lock lock{ m_image2DRegistry.m_mutex };
			for (const auto& resource : m_image2DRegistry.m_resources)
			{
				if (resource.referenceCount == 0)
				{
					continue;
				}

				VT_CORE_TRACE("Image View: {0}", resource.handle.Get());
			}
		}

		VT_CORE_TRACE("-------------------------------------------------");
	}

	void BindlessResourcesManager::Update()
	{
		m_image2DRegistry.Update();
		m_image2DArrayRegistry.Update();
		m_imageCubeRegistry.Update();
		m_samplerRegistry.Update();
		m_bufferRegistry.Update();
	}

	void BindlessResourcesManager::PrepareForRender()
	{
		// Buffers
		{
			std::scoped_lock lock{ m_bufferRegistry.GetMutex() };
			for (const auto& resourceHandle : m_bufferRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_bufferRegistry.GetResource(resourceHandle);
				m_bindlessDescriptorTable->SetBufferView(resourceData.resource.As<RHI::StorageBuffer>()->GetView(), 0, BYTEADDRESSBUFFER_BINDING, resourceHandle.Get());
				m_bindlessDescriptorTable->SetBufferView(resourceData.resource.As<RHI::StorageBuffer>()->GetView(), 0, RWBYTEADDRESSBUFFER_BINDING, resourceHandle.Get());
			}

			m_bufferRegistry.ClearDirtyResources();
		}

		// Image2D
		{
			std::scoped_lock lock{ m_image2DRegistry.GetMutex() };
			for (const auto& resourceHandle : m_image2DRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_image2DRegistry.GetResource(resourceHandle);
				const auto imageView = resourceData.resource.As<RHI::ImageView>();
				
				VT_ENSURE(resourceData.imageUsage != RHI::ImageUsage::None);

				m_bindlessDescriptorTable->SetImageView(imageView, 0, TEXTURE2D_BINDING, resourceHandle.Get());
				if (resourceData.imageUsage == RHI::ImageUsage::Storage || resourceData.imageUsage == RHI::ImageUsage::AttachmentStorage)
				{
					m_bindlessDescriptorTable->SetImageView(imageView, 0, RWTEXTURE2D_BINDING, resourceHandle.Get());
				}
			}

			m_image2DRegistry.ClearDirtyResources();
		}

		// Image2DArray
		{
			std::scoped_lock lock{ m_image2DArrayRegistry.GetMutex() };
			for (const auto& resourceHandle : m_image2DArrayRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_image2DArrayRegistry.GetResource(resourceHandle);
				const auto imageView = resourceData.resource.As<RHI::ImageView>();

				VT_ENSURE(resourceData.imageUsage != RHI::ImageUsage::None);

				m_bindlessDescriptorTable->SetImageView(imageView, 0, TEXTURE2DARRAY_BINDING, resourceHandle.Get());
				if (resourceData.imageUsage == RHI::ImageUsage::Storage || resourceData.imageUsage == RHI::ImageUsage::AttachmentStorage)
				{
					m_bindlessDescriptorTable->SetImageView(imageView, 0, RWTEXTURE2DARRAY_BINDING, resourceHandle.Get());
				}
			}

			m_image2DArrayRegistry.ClearDirtyResources();
		}

		// ImageCube
		{
			std::scoped_lock lock{ m_imageCubeRegistry.GetMutex() };
			for (const auto& resourceHandle : m_imageCubeRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_imageCubeRegistry.GetResource(resourceHandle);
				const auto imageView = resourceData.resource.As<RHI::ImageView>();

				VT_ENSURE(resourceData.imageUsage != RHI::ImageUsage::None);

				m_bindlessDescriptorTable->SetImageView(imageView, 0, TEXTURECUBE_BINDING, resourceHandle.Get());
			}

			m_imageCubeRegistry.ClearDirtyResources();
		}

		// Samplers
		{
			std::scoped_lock lock{ m_samplerRegistry.GetMutex() };
			for (const auto& resourceHandle : m_samplerRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_samplerRegistry.GetResource(resourceHandle);
				const auto samplerState = resourceData.resource.As<RHI::SamplerState>();

				m_bindlessDescriptorTable->SetSamplerState(samplerState, 0, SAMPLERSTATES_BINDING, resourceHandle.Get());
			}

			m_samplerRegistry.ClearDirtyResources();
		}

		m_bindlessDescriptorTable->PrepareForRender();
	}

	ResourceRegistry::ResourceRegistry()
	{
		m_removalQueue.resize(FRAME_COUNT);
	}

	void ResourceRegistry::Update()
	{
		std::scoped_lock lock{ m_mutex };

		m_removalQueue.at(m_frameIndex).Flush();
		m_frameIndex = (m_frameIndex + 1) % FRAME_COUNT;
	}

	void ResourceRegistry::MarkAsDirty(ResourceHandle handle)
	{
		auto it = std::find(m_dirtyResources.begin(), m_dirtyResources.end(), handle);
		if (it == m_dirtyResources.end())
		{
			m_dirtyResources.push_back(handle);
		}
	}

	void ResourceRegistry::ClearDirtyResources()
	{
		m_dirtyResources.clear();
	}

	ResourceHandle ResourceRegistry::RegisterResource(Weak<RHI::RHIInterface> resource, RHI::ImageUsage imageUsage)
	{
		std::scoped_lock lock{ m_mutex };

		const size_t resourceHash = resource.GetHash();
		if (m_resourceHashToHandle.contains(resourceHash))
		{
			const ResourceHandle resourceHandle = m_resourceHashToHandle.at(resourceHash);
			m_resources.at(resourceHandle.Get()).referenceCount++;
			return resourceHandle;
		}

		ResourceHandle newHandle = Resource::Invalid;
		if (!m_vacantResourceHandles.empty())
		{
			newHandle = m_vacantResourceHandles.back();
			m_vacantResourceHandles.pop_back();
		}

		RegisteredResource* registeredResourcePtr = nullptr;

		if (newHandle == Resource::Invalid)
		{
			newHandle = m_currentMaxHandle++;
			registeredResourcePtr = &m_resources.emplace_back();
		}
		else
		{
			registeredResourcePtr = &m_resources.at(newHandle.Get());
		}

		registeredResourcePtr->handle = newHandle;
		registeredResourcePtr->resource = resource;
		registeredResourcePtr->referenceCount = 1;
		registeredResourcePtr->imageUsage = imageUsage;

		m_resourceHashToHandle[resourceHash] = newHandle;
		m_dirtyResources.emplace_back(newHandle);

		return newHandle;
	}

	void ResourceRegistry::UnregisterResource(ResourceHandle handle)
	{
		std::scoped_lock lock{ m_mutex };

		if (handle >= m_resources.size())
		{
			VT_CORE_WARN("[ResourceRegistry]: Resource with handle {0} is not valid!", handle.Get());
			return;
		}

		auto& resource = m_resources.at(handle.Get());
		if (resource.referenceCount > 1)
		{
			resource.referenceCount--;
			return;
		}

		resource.referenceCount = 0;
		m_resourceHashToHandle.erase(resource.resource.GetHash());

		auto it = std::find_if(m_dirtyResources.begin(), m_dirtyResources.end(), [handle](const auto& dirtyHandle)
			{
				return handle == dirtyHandle;
			});

		if (it != m_dirtyResources.end())
		{
			m_dirtyResources.erase(it);
		}

		m_removalQueue.at(m_frameIndex).Push([handle, this]()
			{
				m_vacantResourceHandles.emplace_back(handle);
			});
	}

	ResourceHandle ResourceRegistry::GetResourceHandle(Weak<RHI::RHIInterface> resource)
	{
		return m_resourceHashToHandle.at(resource.GetHash());
	}
}
