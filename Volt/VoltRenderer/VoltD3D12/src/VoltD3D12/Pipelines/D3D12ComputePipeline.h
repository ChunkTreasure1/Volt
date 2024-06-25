#pragma once

#include "VoltD3D12/Common/ComPtr.h"

#include <VoltRHI/Pipelines/ComputePipeline.h>

struct ID3D12PipelineState;

namespace Volt::RHI
{
	class D3D12ComputePipeline : public ComputePipeline
	{
	public:
		D3D12ComputePipeline(RefPtr<Shader> shader, bool useGlobalResources);
		~D3D12ComputePipeline() override;

		void Invalidate() override;
		RefPtr<Shader> GetShader() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();

		ComPtr<ID3D12PipelineState> m_pipeline;

		RefPtr<Shader> m_shader;
		bool m_useGlobalResouces = false;
	};
}
