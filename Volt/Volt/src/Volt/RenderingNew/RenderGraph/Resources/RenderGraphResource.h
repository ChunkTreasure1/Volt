#pragma once

#include "RenderGraphResourceHandle.h"

namespace Volt
{
	namespace RHI
	{
		class Image2D;
		class StorageBuffer;
		class UniformBuffer;
	}

	class RenderGraph;
	struct RenderGraphPassNodeBase;

	struct RenderGraphResourceNodeBase
	{
		uint32_t refCount = 0;
		Weak<RenderGraphPassNodeBase> producer;
		Weak<RenderGraphPassNodeBase> lastUsage;

		RenderGraphResourceHandle handle;

		template<typename T>
		T& As()
		{
			return *reinterpret_cast<T*>(this);
		}
	};

	template<typename T>
	struct RenderGraphResourceNode : public RenderGraphResourceNodeBase
	{
		T resourceInfo;
	};

	class RenderGraphPassResources
	{
	public:
		RenderGraphPassResources(RenderGraph& renderGraph, RenderGraphPassNodeBase& pass);
		
		Ref<RHI::Image2D> GetImage2D(const RenderGraphResourceHandle resourceHandle);
		//Ref<RHI::Image3D> GetImage3D(const RenderGraphResourceHandle resourceHandle); // #TODO: Implement Image3D first
		Ref<RHI::StorageBuffer> GetBuffer(const RenderGraphResourceHandle resourceHandle);
		Ref<RHI::UniformBuffer> GetUniformBuffer(const RenderGraphResourceHandle resourceHandle);

	private:
		void ValidateResourceAccess(const RenderGraphResourceHandle resourceHandle);

		RenderGraph& m_renderGraph;
		RenderGraphPassNodeBase& m_pass;
	};
}
