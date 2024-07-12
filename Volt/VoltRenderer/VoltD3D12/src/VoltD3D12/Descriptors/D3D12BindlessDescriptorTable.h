#pragma once

#include "VoltD3D12/Descriptors/DescriptorCommon.h"

#include <VoltRHI/Descriptors/BindlessDescriptorTable.h>
#include <VoltRHI/Descriptors/ResourceRegistry.h>

#include <VoltRHI/Descriptors/ResourceRegistry.h>

#include <CoreUtilities/Containers/Map.h>

namespace Volt::RHI
{
	class D3D12DescriptorHeap;
	class D3D12BindlessDescriptorTable : public BindlessDescriptorTable
	{
	public:
		D3D12BindlessDescriptorTable();
		~D3D12BindlessDescriptorTable() override;

		ResourceHandle RegisterBuffer(WeakPtr<StorageBuffer> storageBuffer) override;
		ResourceHandle RegisterImageView(WeakPtr<ImageView> imageView) override;
		ResourceHandle RegisterSamplerState(WeakPtr<SamplerState> samplerState) override;

		void UnregisterBuffer(ResourceHandle handle) override;
		void UnregisterImageView(ResourceHandle handle, ImageViewType viewType) override;
		void UnregisterSamplerState(ResourceHandle handle) override;

		void MarkBufferAsDirty(ResourceHandle handle) override;
		void MarkImageViewAsDirty(ResourceHandle handle, RHI::ImageViewType viewType) override;
		void MarkSamplerStateAsDirty(ResourceHandle handle) override;

		ResourceHandle GetBufferHandle(WeakPtr<RHI::StorageBuffer> storageBuffer) override;

		void Update() override;
		void PrepareForRender() override;

		void Bind(CommandBuffer& commandBuffer) override;
		void SetRootDescriptorTables(CommandBuffer& commandBuffer);

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();
		void Invalidate();

		void CreateDescriptorHeaps();

		ResourceRegistry m_mainRegistry;
		ResourceRegistry m_samplerRegistry;

		std::vector<DescriptorCopyInfo> m_activeDescriptorCopies;
		std::vector<DescriptorCopyInfo> m_activeSamplerDescriptorCopies;

		vt::map<ResourceHandle, D3D12DescriptorPointer> m_allocatedDescriptorPointers;
		vt::map<ResourceHandle, D3D12DescriptorPointer> m_allocatedSamplerDescriptorPointers;

		Scope<D3D12DescriptorHeap> m_mainHeap;
		Scope<D3D12DescriptorHeap> m_samplerHeap;
	};
}
