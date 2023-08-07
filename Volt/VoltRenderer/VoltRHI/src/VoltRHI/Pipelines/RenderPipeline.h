#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	class Shader;

	struct RenderPipelineCreateInfo
	{
		Ref<Shader> shader;

		Topology topology = Topology::TriangleList;
		CullMode cullMode = CullMode::Back;
		FillMode fillMode = FillMode::Solid;
		DepthMode depthMode = DepthMode::ReadWrite;
		CompareOperator depthCompareOperator = CompareOperator::GreaterEqual;

		std::string name;
	};

	class RenderPipeline : public RHIInterface
	{
	public:
		virtual void Invalidate() = 0;

		static Ref<RenderPipeline> Create(const RenderPipelineCreateInfo& createInfo);

	protected:
		RenderPipeline() = default;
		virtual ~RenderPipeline() = default;
	};
}
