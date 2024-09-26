#pragma once

#include "D3D12RHIModule/Common/ComPtr.h"

#include <RHIModule/Pipelines/RenderPipeline.h>

struct ID3D12PipelineState;

namespace Volt::RHI
{
	class D3D12RenderPipeline final : public RenderPipeline
	{
	public:
		D3D12RenderPipeline(const RenderPipelineCreateInfo& createInfo);
		~D3D12RenderPipeline() override;

		void Invalidate() override;

		RefPtr<Shader> GetShader() const override;
		VT_NODISCARD VT_INLINE Topology GetTopology() const { return m_createInfo.topology; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();

		RenderPipelineCreateInfo m_createInfo;
		ComPtr<ID3D12PipelineState> m_pipeline;
	};
}
