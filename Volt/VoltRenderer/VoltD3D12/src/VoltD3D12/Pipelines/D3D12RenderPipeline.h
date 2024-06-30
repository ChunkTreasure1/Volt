#pragma once

#include "VoltD3D12/Common/ComPtr.h"

#include <VoltRHI/Pipelines/RenderPipeline.h>

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

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();

		RenderPipelineCreateInfo m_createInfo;
		ComPtr<ID3D12PipelineState> m_pipeline;
	};
}
