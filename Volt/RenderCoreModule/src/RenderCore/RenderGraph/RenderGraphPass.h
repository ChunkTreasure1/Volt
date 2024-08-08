#pragma once

#include "RenderCore/Config.h"

#include "RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphResource.h"

#include <string_view>


namespace Volt
{
	class RenderGraph;
	class RenderContext;

	enum class RenderGraphResourceState
	{
		None = 0,
		IndirectArgument,
		IndexBuffer,
		VertexBuffer,
		CopyDest,
		CopySource,
		Clear
	};

	struct RenderGraphPassResourceAccess
	{ 
		RenderGraphResourceState forcedState = RenderGraphResourceState::None;
		RenderGraphResourceHandle handle;
	};

	struct RenderGraphPassNodeBase
	{
		std::string name;
		
		uint32_t index = 0;
		uint32_t refCount = 0;

		bool isComputePass = false;
		bool isCulled = false;
		bool hasSideEffect = false;

		Vector<RenderGraphPassResourceAccess> resourceReads;
		Vector<RenderGraphPassResourceAccess> resourceWrites;
		Vector<RenderGraphPassResourceAccess> resourceCreates;

		virtual void Execute(RenderGraph& frameGraph, RenderContext& context) = 0;

		const bool ReadsResource(RenderGraphResourceHandle handle) const;
		const bool WritesResource(RenderGraphResourceHandle handle) const;
		const bool CreatesResource(RenderGraphResourceHandle handle) const;
		const bool IsCulled() const;

#ifdef VT_DEBUG
		const bool ReadsResource(ResourceHandle handle) const;
		const bool WritesResource(ResourceHandle handle) const;
		const bool CreatesResource(ResourceHandle handle) const;
#endif

	private:
		friend class RenderGraphPassResources;

#ifdef VT_DEBUG
		std::unordered_map<ResourceHandle, RenderGraphResourceHandle> m_resourceHandleMapping;
#endif
	};

	template<typename T>
	struct RenderGraphPassNode : public RenderGraphPassNodeBase
	{
		T data{};
		std::function<void(const T& data, RenderContext& context)> executeFunction;

		void Execute(RenderGraph& renderGraph, RenderContext& context) override
		{
			RenderGraphPassResources resources{ renderGraph, *this };
			executeFunction(data, context);
		}
	};
}
