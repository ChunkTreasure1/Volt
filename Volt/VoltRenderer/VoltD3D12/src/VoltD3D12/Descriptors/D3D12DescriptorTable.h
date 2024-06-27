#pragma once

#include "VoltD3D12/Common/ComPtr.h"
#include "VoltD3D12/Descriptors/DescriptorCommon.h"
#include "VoltD3D12/Common/D3D12Common.h"

#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <VoltRHI/Shader/Shader.h>

#include <CoreUtilities/Containers/Map.h>

namespace Volt::RHI
{
	class D3D12DescriptorHeap;
	class D3D12DescriptorTable : public DescriptorTable
	{
	public:
		D3D12DescriptorTable(const DescriptorTableCreateInfo& createInfo);
		~D3D12DescriptorTable() override;

		void SetImageView(WeakPtr<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;
		void SetBufferView(WeakPtr<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;
		void SetSamplerState(WeakPtr<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex /* = 0 */) override;

		void SetImageView(std::string_view name, WeakPtr<ImageView> view, uint32_t arrayIndex = 0) override;
		void SetBufferView(std::string_view name, WeakPtr<BufferView> view, uint32_t arrayIndex = 0) override;
		void SetSamplerState(std::string_view name, WeakPtr<SamplerState> samplerState, uint32_t arrayIndex = 0) override;

		void PrepareForRender() override;
		void Bind(CommandBuffer& commandBuffer) override;

	protected:
		void* GetHandleImpl() const override;

	private:
		struct DescriptorCopyInfo
		{
			D3D12DescriptorPointer srcPointer;
			D3D12DescriptorPointer dstPointer;
		};
		
		struct AllocatedDescriptorInfo
		{
			D3D12DescriptorPointer pointer;
			D3D12ViewType viewType;
		};

		void Invalidate();
		void Release();

		void CreateDescriptorHeaps(uint32_t mainDescriptorCount, uint32_t samplerDescriptorCount);
		void AllocateDescriptors();

		WeakPtr<Shader> m_shader;
		bool m_isDirty = false;

		vt::map<uint32_t, vt::map<uint32_t, AllocatedDescriptorInfo>> m_allocatedDescriptorPointers;
		std::vector<DescriptorCopyInfo> m_activeDescriptorCopies;
		std::vector<DescriptorCopyInfo> m_activeSamplerDescriptorCopies;

		Scope<D3D12DescriptorHeap> m_mainHeap;
		Scope<D3D12DescriptorHeap> m_samplerHeap;
	};
}
