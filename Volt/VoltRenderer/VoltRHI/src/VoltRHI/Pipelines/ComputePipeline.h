#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	class Shader;

	class VTRHI_API ComputePipeline : public RHIInterface
	{
	public:
		virtual void Invalidate() = 0;
		virtual RefPtr<Shader> GetShader() const = 0;

		static RefPtr<ComputePipeline> Create(RefPtr<Shader> shader, bool useGlobalResources = true);

	protected:
		ComputePipeline() = default;
		virtual ~ComputePipeline() = default;
	};
}

