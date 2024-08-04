#pragma once

#include "D3D12RHIModule/Common/ComPtr.h"
#include "D3D12RHIModule/Descriptors/D3D12DescriptorHeap.h"

#include <RHIModule/ImGui/ImGuiImplementation.h>
#include <CoreUtilities/Containers/Map.h>

struct ID3D12DescriptorHeap;

namespace Volt::RHI
{
	class CommandBuffer;
	class D3D12ImGuiImplementation final : public ImGuiImplementation
	{
	public:
		D3D12ImGuiImplementation(const ImGuiCreateInfo& createInfo);
		~D3D12ImGuiImplementation() override;
		
		ImTextureID GetTextureID(RefPtr<Image2D> image) const override;
		ImFont* AddFont(const std::filesystem::path& fontPath, float pixelSize) override;

	protected:
		void BeginAPI() override;
		void EndAPI() override;

		void InitializeAPI(ImGuiContext* context) override;
		void ShutdownAPI() override;

		void* GetHandleImpl() const override;
	private:
		ImGuiCreateInfo m_info;

		D3D12DescriptorPointer m_fontTexturePointer;
		RefPtr<CommandBuffer> m_commandBuffer;

		uint32_t m_currentFrameIndex = 0;
		const uint32_t m_framesInFlight = 0;

		Scope<D3D12DescriptorHeap> m_descriptorHeap;
		mutable Vector<vt::map<size_t, D3D12DescriptorPointer>> m_descriptorCache;
	};
}
