#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	class Shader;

	class VTRHI_API ComputePipeline : public RHIInterface
	{
	public:
		virtual void Invalidate() = 0;
		virtual Ref<Shader> GetShader() const = 0;

		static Ref<ComputePipeline> Create(Ref<Shader> shader, bool useGlobalResources = true);

	protected:
		ComputePipeline() = default;
		virtual ~ComputePipeline() = default;
	};
}

