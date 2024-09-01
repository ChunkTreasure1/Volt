#pragma once

#include "RHIModule/Core/RHIInterface.h"
#include "RHIModule/Core/RHICommon.h"

namespace Volt::RHI
{
	class Shader;

	struct RenderPipelineCreateInfo
	{
		RefPtr<Shader> shader;

		Topology topology = Topology::TriangleList;
		CullMode cullMode = CullMode::Back;
		FillMode fillMode = FillMode::Solid;
		DepthMode depthMode = DepthMode::ReadWrite;
		CompareOperator depthCompareOperator = CompareOperator::GreaterEqual;
		bool enablePrimitiveRestart = false;

		std::string name;
	};

	class VTRHI_API RenderPipeline : public RHIInterface
	{
	public:
		virtual void Invalidate() = 0;
		virtual RefPtr<Shader> GetShader() const = 0;

		static RefPtr<RenderPipeline> Create(const RenderPipelineCreateInfo& createInfo);

	protected:
		RenderPipeline() = default;
		virtual ~RenderPipeline() = default;
	};
}
