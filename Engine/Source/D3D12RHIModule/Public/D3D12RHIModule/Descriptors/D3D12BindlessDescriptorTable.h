#pragma once

#include "D3D12RHIModule/Descriptors/DescriptorCommon.h"

#include <RHIModule/Descriptors/BindlessDescriptorTable.h>
#include <RHIModule/Descriptors/ResourceRegistry.h>

#include <RHIModule/Descriptors/ResourceRegistry.h>

#include <CoreUtilities/Containers/Map.h>

namespace Volt::RHI
{
	class D3D12DescriptorHeap;
	class D3D12BindlessDescriptorTable : public BindlessDescriptorTable
	{
	public:
		D3D12BindlessDescriptorTable(const uint64_t framesInFlight);
		~D3D12BindlessDescriptorTable() override;

		ResourceHandle RegisterBuffer(WeakPtr<StorageBuffer> storageBuffer) override;
		ResourceHandle RegisterImageView(WeakPtr<ImageView> imageView) override;
		ResourceHandle RegisterSamplerState(WeakPtr<SamplerState> samplerState) override;

		void UnregisterResource(ResourceHandle handle) override;
		void MarkResourceAsDirty(ResourceHandle handle) override;

		void UnregisterSamplerState(ResourceHandle handle) override;
		void MarkSamplerStateAsDirty(ResourceHandle handle) override;

		void Update() override;
		void PrepareForRender() override;

		void Bind(CommandBuffer& commandBuffer, WeakPtr<UniformBuffer> constantsBuffer, const uint32_t offsetIndex, const uint32_t stride) override;
		void SetRootParameters(CommandBuffer& commandBuffer, WeakPtr<UniformBuffer> constantsBuffer);

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();
		void Invalidate();

		void CreateDescriptorHeaps();

		ResourceRegistry m_mainRegistry;
		ResourceRegistry m_samplerRegistry;

		Vector<DescriptorCopyInfo> m_activeDescriptorCopies;
		Vector<DescriptorCopyInfo> m_activeSamplerDescriptorCopies;

		vt::map<ResourceHandle, D3D12DescriptorPointer> m_allocatedDescriptorPointers;
		vt::map<ResourceHandle, D3D12DescriptorPointer> m_allocatedSamplerDescriptorPointers;

		Scope<D3D12DescriptorHeap> m_mainHeap;
		Scope<D3D12DescriptorHeap> m_samplerHeap;

		uint32_t m_offsetIndex = 0;
		uint32_t m_offsetStride = 0;
	};
}
