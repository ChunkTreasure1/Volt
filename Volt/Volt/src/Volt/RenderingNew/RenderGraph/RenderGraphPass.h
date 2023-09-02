#pragma once

#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResource.h"

#include <string_view>
#include <vector>

namespace Volt
{
	class RenderGraph;

	struct RenderGraphPassNodeBase
	{
		std::string_view name;
		
		uint32_t passIndex = 0;
		uint32_t refCount = 0;

		bool isComputePass = false;
		bool isCulled = false;
		bool hasSideEffect = false;

		std::vector<RenderGraphResourceHandle> resourceReads;
		std::vector<RenderGraphResourceHandle> resourceWrites;
		std::vector<RenderGraphResourceHandle> resourceCreates;

		virtual void Execute(RenderGraph& frameGraph) = 0;

		const bool ReadsResource(RenderGraphResourceHandle handle) const;
		const bool WritesResource(RenderGraphResourceHandle handle) const;
		const bool CreatesResource(RenderGraphResourceHandle handle) const;
		const bool IsCulled() const;
	};

	template<typename T>
	struct RenderGraphPassNode : public RenderGraphPassNodeBase
	{
		T data{};
		std::function<void(const T& data, const RenderGraphPassResources& resources)> executeFunction;

		void Execute(RenderGraph& renderGraph) override
		{
			RenderGraphPassResources resources{ renderGraph, *this };
			executeFunction(data, resources);
		}
	};
}
