#pragma once

#include "VoltRHI/Core/RHIInterface.h"

namespace Volt::RHI
{
	class Shader;

	class ComputePipeline : public RHIInterface
	{
	public:
		virtual void Invalidate() = 0;

		static Ref<ComputePipeline> Create(Ref<Shader> shader);

	protected:
		ComputePipeline() = default;
		virtual ~ComputePipeline() = default;
	};
}

