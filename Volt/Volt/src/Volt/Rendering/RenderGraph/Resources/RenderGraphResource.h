#pragma once

#include "RenderGraphResourceHandle.h"

#include <VoltRHI/Descriptors/ResourceHandle.h>
#include <VoltRHI/Core/RHICommon.h>

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

	enum class ResourceType
	{
		Image2D,
		Image3D,
		Buffer,
		UniformBuffer
	};

	struct RenderGraphBarrierInfo
	{
		RHI::BarrierStage dstStage;
		RHI::BarrierAccess dstAccess;

		// Only images
		RHI::ImageLayout dstLayout;
	};

	struct RenderGraphResourceNodeBase
	{
		virtual ~RenderGraphResourceNodeBase() = default;

		uint32_t refCount = 0;
		size_t hash = 0;

		Weak<RenderGraphPassNodeBase> producer;
		Weak<RenderGraphPassNodeBase> lastUsage;

		RenderGraphResourceHandle handle;

		bool isExternal = false;

		virtual ResourceType GetResourceType() const = 0;

		template<typename T>
		T& As()
		{
			return *reinterpret_cast<T*>(this);
		}
	};

	template<typename T>
	struct RenderGraphResourceNode : public RenderGraphResourceNodeBase
	{
		RenderGraphResourceNode() = default;
		~RenderGraphResourceNode() override = default;

		T resourceInfo;

		constexpr ResourceType GetResourceType() const override { return T::GetResourceType(); }
	};

	class RenderGraphPassResources
	{
	public:
		RenderGraphPassResources(RenderGraph& renderGraph, RenderGraphPassNodeBase& pass);
		
		ResourceHandle GetImage2D(const RenderGraphResourceHandle resourceHandle, const uint32_t mip = 0, const uint32_t layer = 0) const;
		//RefPtr<RHI::Image3D> GetImage3D(const RenderGraphResourceHandle resourceHandle); // #TODO: Implement Image3D first
		ResourceHandle GetBuffer(const RenderGraphResourceHandle resourceHandle) const;
		ResourceHandle GetUniformBuffer(const RenderGraphResourceHandle resourceHandle) const;

	private:
		friend class RenderContext;

		void ValidateResourceAccess(const RenderGraphResourceHandle resourceHandle) const;

		RenderGraph& m_renderGraph;
		RenderGraphPassNodeBase& m_pass;
	};

	struct RenderGraphResourceAccess
	{
		RHI::BarrierStage dstStage = RHI::BarrierStage::None;
		RHI::BarrierAccess dstAccess = RHI::BarrierAccess::None;
		
		// Image only
		RHI::ImageLayout dstLayout = RHI::ImageLayout::Undefined;

		RenderGraphResourceHandle resourceHandle = std::numeric_limits<RenderGraphResourceHandle>::max();
 	};
}
