#include "dxpch.h"
#include "D3D12BindlessDescriptorTable.h"

#include "VoltD3D12/Descriptors/D3D12DescriptorHeap.h"
#include "VoltD3D12/Buffers/D3D12CommandBuffer.h"
#include "VoltD3D12/Buffers/D3D12BufferView.h"
#include "VoltD3D12/Images/D3D12ImageView.h"

#include <VoltRHI/Pipelines/RenderPipeline.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>

#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

#include <VoltRHI/Shader/Shader.h>

namespace Volt::RHI
{
	D3D12BindlessDescriptorTable::D3D12BindlessDescriptorTable()
		: m_mainRegistry(2), m_samplerRegistry(1)
	{
		m_activeDescriptorCopies.reserve(100);
		m_activeSamplerDescriptorCopies.reserve(10);
		
		Invalidate();
	}

	D3D12BindlessDescriptorTable::~D3D12BindlessDescriptorTable()
	{
		Release();
	}

	ResourceHandle D3D12BindlessDescriptorTable::RegisterBuffer(WeakPtr<StorageBuffer> storageBuffer)
	{
		VT_PROFILE_FUNCTION();
		return m_mainRegistry.RegisterResource(storageBuffer, ImageUsage::None, static_cast<uint32_t>(ResourceType::StorageBuffer));
	}

	ResourceHandle D3D12BindlessDescriptorTable::RegisterImageView(WeakPtr<ImageView> imageView)
	{
		VT_PROFILE_FUNCTION();
		return m_mainRegistry.RegisterResource(imageView, imageView->GetImageUsage(), static_cast<uint32_t>(ResourceType::Image2D));
	}

	ResourceHandle D3D12BindlessDescriptorTable::RegisterSamplerState(WeakPtr<SamplerState> samplerState)
	{
		VT_PROFILE_FUNCTION();
		return m_samplerRegistry.RegisterResource(samplerState);
	}

	void D3D12BindlessDescriptorTable::UnregisterBuffer(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_mainRegistry.UnregisterResource(handle);
	}

	void D3D12BindlessDescriptorTable::UnregisterImageView(ResourceHandle handle, ImageViewType viewType)
	{
		VT_PROFILE_FUNCTION();
		m_mainRegistry.UnregisterResource(handle);
	}

	void D3D12BindlessDescriptorTable::UnregisterSamplerState(ResourceHandle handle)
	{
		m_samplerRegistry.UnregisterResource(handle);
	}

	void D3D12BindlessDescriptorTable::MarkBufferAsDirty(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_mainRegistry.MarkAsDirty(handle);
	}

	void D3D12BindlessDescriptorTable::MarkImageViewAsDirty(ResourceHandle handle, RHI::ImageViewType viewType)
	{
		VT_PROFILE_FUNCTION();
		m_mainRegistry.MarkAsDirty(handle);
	}

	void D3D12BindlessDescriptorTable::MarkSamplerStateAsDirty(ResourceHandle handle)
	{
		VT_PROFILE_FUNCTION();
		m_samplerRegistry.MarkAsDirty(handle);
	}

	ResourceHandle D3D12BindlessDescriptorTable::GetBufferHandle(WeakPtr<RHI::StorageBuffer> storageBuffer)
	{
		VT_PROFILE_FUNCTION();
		return m_mainRegistry.GetResourceHandle(storageBuffer);
	}

	void D3D12BindlessDescriptorTable::Update()
	{
		VT_PROFILE_FUNCTION();
		m_mainRegistry.Update();
		m_samplerRegistry.Update();
	}

	void D3D12BindlessDescriptorTable::PrepareForRender()
	{
		// Main heap
		{
			std::scoped_lock lock{ m_mainRegistry.GetMutex() };
			for (const auto& resourceHandle : m_mainRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_mainRegistry.GetResource(resourceHandle);
				const ResourceType resourceType = static_cast<ResourceType>(resourceData.userData);

				if (resourceType == ResourceType::StorageBuffer)
				{
					auto view = resourceData.resource.As<StorageBuffer>()->GetView().As<D3D12BufferView>();

					// Read Only
					{
						auto& descriptorCopy = m_activeDescriptorCopies.emplace_back();
						if (m_allocatedDescriptorPointers.contains(resourceHandle))
						{
							descriptorCopy.dstPointer = m_allocatedDescriptorPointers.at(resourceHandle);
						}
						else
						{
							descriptorCopy.dstPointer = m_mainHeap->Allocate(resourceHandle.Get());
							m_allocatedDescriptorPointers[resourceHandle] = descriptorCopy.dstPointer;
						}

						descriptorCopy.srcPointer = view->GetSRVDescriptor();

						VT_ENSURE(descriptorCopy.dstPointer.IsValid());
						VT_ENSURE(descriptorCopy.srcPointer.IsValid());
					}

					// Read-Write
					if (view->GetUAVDescriptor().IsValid())
					{
						auto writeHandle = resourceHandle + ResourceHandle(1u);
						auto& descriptorCopy = m_activeDescriptorCopies.emplace_back();
					
						if (m_allocatedDescriptorPointers.contains(writeHandle))
						{
							descriptorCopy.dstPointer = m_allocatedDescriptorPointers.at(writeHandle);
						}
						else
						{
							descriptorCopy.dstPointer = m_mainHeap->Allocate(writeHandle.Get());
							m_allocatedDescriptorPointers[writeHandle] = descriptorCopy.dstPointer;
						}

						descriptorCopy.srcPointer = view->GetUAVDescriptor();

						VT_ENSURE(descriptorCopy.dstPointer.IsValid());
						VT_ENSURE(descriptorCopy.srcPointer.IsValid());
					}
				}
				else if (resourceType == ResourceType::Image2D)
				{
					auto view = resourceData.resource.As<D3D12ImageView>();

					// Read Only
					{
						auto& descriptorCopy = m_activeDescriptorCopies.emplace_back();
						if (m_allocatedDescriptorPointers.contains(resourceHandle))
						{
							descriptorCopy.dstPointer = m_allocatedDescriptorPointers.at(resourceHandle);
						}
						else
						{
							descriptorCopy.dstPointer = m_mainHeap->Allocate(resourceHandle.Get());
							m_allocatedDescriptorPointers[resourceHandle] = descriptorCopy.dstPointer;
						}

						descriptorCopy.srcPointer = view->GetSRVDescriptor();

						VT_ENSURE(descriptorCopy.dstPointer.IsValid());
						VT_ENSURE(descriptorCopy.srcPointer.IsValid());
					}

					// Read-Write
					if (resourceData.imageUsage == RHI::ImageUsage::Storage || resourceData.imageUsage == RHI::ImageUsage::AttachmentStorage)
					{
						auto writeHandle = resourceHandle + ResourceHandle(1u);
						auto& descriptorCopy = m_activeDescriptorCopies.emplace_back();

						if (m_allocatedDescriptorPointers.contains(writeHandle))
						{
							descriptorCopy.dstPointer = m_allocatedDescriptorPointers.at(writeHandle);
						}
						else
						{
							descriptorCopy.dstPointer = m_mainHeap->Allocate(writeHandle.Get());
							m_allocatedDescriptorPointers[writeHandle] = descriptorCopy.dstPointer;
						}

						descriptorCopy.srcPointer = view->GetUAVDescriptor();

						VT_ENSURE(descriptorCopy.dstPointer.IsValid());
						VT_ENSURE(descriptorCopy.srcPointer.IsValid());
					}
				}
			}
			m_mainRegistry.ClearDirtyResources();
		}

		// Samplers
		{
			std::scoped_lock lock{ m_mainRegistry.GetMutex() };
			for (const auto& resourceHandle : m_mainRegistry.GetDirtyResources())
			{
				const auto& resourceData = m_mainRegistry.GetResource(resourceHandle);

				auto& descriptorCopy = m_activeSamplerDescriptorCopies.emplace_back();
				if (m_allocatedSamplerDescriptorPointers.contains(resourceHandle))
				{
					descriptorCopy.dstPointer = m_allocatedSamplerDescriptorPointers.at(resourceHandle);
				}
				else
				{
					descriptorCopy.dstPointer = m_samplerHeap->Allocate(resourceHandle.Get());
					m_allocatedSamplerDescriptorPointers[resourceHandle] = descriptorCopy.dstPointer;
				}

				descriptorCopy.srcPointer = *resourceData.resource->GetHandle<D3D12DescriptorPointer*>();

				VT_ENSURE(descriptorCopy.dstPointer.IsValid());
				VT_ENSURE(descriptorCopy.srcPointer.IsValid());
			}
			m_samplerRegistry.ClearDirtyResources();
		}

		if (m_activeDescriptorCopies.empty() && m_activeSamplerDescriptorCopies.empty())
		{
			return;
		}

		auto d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();

		if (!m_activeDescriptorCopies.empty())
		{
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> srcHandles(m_activeDescriptorCopies.size());
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> dstHandles(m_activeDescriptorCopies.size());
			std::vector<uint32_t> copySizes(m_activeDescriptorCopies.size(), 1u);

			for (size_t i = 0; i < m_activeDescriptorCopies.size(); i++)
			{
				srcHandles[i] = D3D12_CPU_DESCRIPTOR_HANDLE(m_activeDescriptorCopies.at(i).srcPointer.GetCPUPointer());
				dstHandles[i] = D3D12_CPU_DESCRIPTOR_HANDLE(m_activeDescriptorCopies.at(i).dstPointer.GetCPUPointer());
			}

			d3d12Device->CopyDescriptors(static_cast<uint32_t>(m_activeDescriptorCopies.size()), dstHandles.data(), copySizes.data(), static_cast<uint32_t>(m_activeDescriptorCopies.size()), srcHandles.data(), copySizes.data(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		if (!m_activeSamplerDescriptorCopies.empty())
		{
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> srcHandles(m_activeSamplerDescriptorCopies.size());
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> dstHandles(m_activeSamplerDescriptorCopies.size());
			std::vector<uint32_t> copySizes(m_activeSamplerDescriptorCopies.size(), 1u);

			for (size_t i = 0; i < m_activeSamplerDescriptorCopies.size(); i++)
			{
				srcHandles[i] = D3D12_CPU_DESCRIPTOR_HANDLE(m_activeSamplerDescriptorCopies.at(i).srcPointer.GetCPUPointer());
				dstHandles[i] = D3D12_CPU_DESCRIPTOR_HANDLE(m_activeSamplerDescriptorCopies.at(i).dstPointer.GetCPUPointer());
			}

			d3d12Device->CopyDescriptors(static_cast<uint32_t>(m_activeSamplerDescriptorCopies.size()), dstHandles.data(), copySizes.data(), static_cast<uint32_t>(m_activeSamplerDescriptorCopies.size()), srcHandles.data(), copySizes.data(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		}

		m_activeDescriptorCopies.clear();
		m_activeSamplerDescriptorCopies.clear();
	}

	void D3D12BindlessDescriptorTable::Bind(CommandBuffer& commandBuffer)
	{
		auto& d3d12CommandBuffer = commandBuffer.AsRef<D3D12CommandBuffer>();
		ID3D12GraphicsCommandList* cmdList = commandBuffer.GetHandle<ID3D12GraphicsCommandList*>();
		ID3D12DescriptorHeap* heaps[2] = { m_mainHeap->GetHeap().Get(), m_samplerHeap->GetHeap().Get() };
	
		cmdList->SetDescriptorHeaps(2, heaps);

		bool hasRootConstants = false;

		if (d3d12CommandBuffer.m_currentRenderPipeline)
		{
			hasRootConstants = d3d12CommandBuffer.m_currentRenderPipeline->GetShader()->HasConstants();
		}
		else if (d3d12CommandBuffer.m_currentComputePipeline)
		{
			hasRootConstants = d3d12CommandBuffer.m_currentComputePipeline->GetShader()->HasConstants();
		}

		uint32_t tableCount = hasRootConstants ? 1 : 0;

		if (d3d12CommandBuffer.m_currentRenderPipeline)
		{
			cmdList->SetGraphicsRootDescriptorTable(tableCount++, m_mainHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart());
			cmdList->SetGraphicsRootDescriptorTable(tableCount++, m_mainHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart());
		}
		else
		{
			cmdList->SetComputeRootDescriptorTable(tableCount++, m_mainHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart());
			cmdList->SetComputeRootDescriptorTable(tableCount++, m_mainHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart());
		}
	}

	void* D3D12BindlessDescriptorTable::GetHandleImpl() const
	{
		return nullptr;
	}

	void D3D12BindlessDescriptorTable::Release()
	{
		m_mainHeap = nullptr;
		m_samplerHeap = nullptr;
	}

	void D3D12BindlessDescriptorTable::Invalidate()
	{
		CreateDescriptorHeaps();
	}

	void D3D12BindlessDescriptorTable::CreateDescriptorHeaps()
	{
		{
			DescriptorHeapSpecification specification{};
			specification.descriptorType = D3D12DescriptorType::CBV_SRV_UAV;
			specification.maxDescriptorCount = 16384;
			specification.supportsGPUDescriptors = true;

			m_mainHeap = CreateScope<D3D12DescriptorHeap>(specification);
		}

		{
			DescriptorHeapSpecification specification{};
			specification.descriptorType = D3D12DescriptorType::Sampler;
			specification.maxDescriptorCount = 100;
			specification.supportsGPUDescriptors = true;

			m_samplerHeap = CreateScope<D3D12DescriptorHeap>(specification);
		}
	}
}
