#pragma once

#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"
#include "RenderGraphResourceHandle.h"

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

	struct RenderGraphResourceNodeBase
	{
		virtual ~RenderGraphResourceNodeBase() = default;

		uint32_t refCount = 0;
		Weak<RenderGraphPassNodeBase> producer;
		Weak<RenderGraphPassNodeBase> lastUsage;

		RenderGraphResourceHandle handle;
		RHI::ResourceState currentState = RHI::ResourceState::Undefined;

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
		
		Weak<RHI::ImageView> GetImage2DView(const RenderGraphResourceHandle resourceHandle) const;
		Weak<RHI::Image2D> GetImage2DRaw(const RenderGraphResourceHandle resourceHandle) const;
		ResourceHandle GetImage2D(const RenderGraphResourceHandle resourceHandle, const uint32_t mip = 0, const uint32_t layer = 0) const;
		
		//Ref<RHI::Image3D> GetImage3D(const RenderGraphResourceHandle resourceHandle); // #TODO: Implement Image3D first

		Weak<RHI::StorageBuffer> GetBufferRaw(const RenderGraphResourceHandle resourceHandle) const;
		ResourceHandle GetBuffer(const RenderGraphResourceHandle resourceHandle) const;
		ResourceHandle GetUniformBuffer(const RenderGraphResourceHandle resourceHandle) const;

	private:
		void ValidateResourceAccess(const RenderGraphResourceHandle resourceHandle) const;

		RenderGraph& m_renderGraph;
		RenderGraphPassNodeBase& m_pass;
	};

	struct RenderGraphResourceAccess
	{
		RHI::ResourceState oldState;
		RHI::ResourceState newState;

		RenderGraphResourceHandle resourceHandle = std::numeric_limits<RenderGraphResourceHandle>::max();
 	};
}
