#pragma once
#include "VoltRHI/Pipelines/RenderPipeline.h"

struct ID3D12RootSignature;
struct ID3D12PipelineState;

namespace Volt::RHI
{
	class D3D12RenderPipeline final : public RenderPipeline
	{
	public:
		D3D12RenderPipeline(const RenderPipelineCreateInfo& createInfo);
		~D3D12RenderPipeline() override;

		ID3D12RootSignature* GetRoot() { return m_rootSignature; }
		ID3D12PipelineState* GetPSO() { return m_pipelineStateObject; }
		Topology GetTopology() { return m_topology; }

		virtual void* GetHandleImpl() const override;
		virtual void Invalidate() override;
		Ref<Shader> GetShader() const override;
	private:
		RenderPipelineCreateInfo m_createInfo;

		ID3D12RootSignature* m_rootSignature;
		ID3D12PipelineState* m_pipelineStateObject;
		Topology m_topology;

		void Create(const RenderPipelineCreateInfo& createInfo);
	};
}
