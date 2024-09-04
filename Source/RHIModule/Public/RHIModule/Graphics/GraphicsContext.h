#pragma once

#include "RHIModule/Core/Core.h"
#include "RHIModule/Core/RHICommon.h"
#include "RHIModule/Core/RHIInterface.h"

#include "RHIModule/Memory/Allocator.h"
#include "RHIModule/Graphics/GraphicsDevice.h"
#include "RHIModule/Graphics/PhysicalGraphicsDevice.h"

#include "RHIModule/Core/ResourceStateTracker.h"

namespace Volt::RHI
{
	class GraphicsDevice;
	class PhysicalGraphicsDevice;

	class VTRHI_API GraphicsContext : public RHIInterface
	{
	public:
		GraphicsContext();
		virtual ~GraphicsContext();

		VT_NODISCARD VT_INLINE static GraphicsContext& Get() { return *s_context; }
		VT_NODISCARD VT_INLINE static RefPtr<GraphicsDevice> GetDevice() { return s_context->GetGraphicsDevice(); };
		VT_NODISCARD VT_INLINE static RefPtr<PhysicalGraphicsDevice> GetPhysicalDevice() { return s_context->GetPhysicalGraphicsDevice(); };
		VT_NODISCARD VT_INLINE static RefPtr<Allocator> GetDefaultAllocator() { return s_context->GetDefaultAllocatorImpl(); };
		VT_NODISCARD VT_INLINE static RefPtr<Allocator> GetTransientAllocator() { return s_context->GetTransientAllocatorImpl(); }
		VT_NODISCARD VT_INLINE static RefPtr<ResourceStateTracker> GetResourceStateTracker() { return s_context->GetResourceStateTrackerImpl(); }
		VT_NODISCARD VT_INLINE static GraphicsAPI GetAPI() { return s_graphicsAPI; }

		static RefPtr<GraphicsContext> Create(const GraphicsContextCreateInfo& createInfo);

		static void Update();

	protected:
		virtual RefPtr<Allocator> GetDefaultAllocatorImpl() = 0;
		virtual RefPtr<Allocator> GetTransientAllocatorImpl() = 0;
		virtual RefPtr<ResourceStateTracker> GetResourceStateTrackerImpl() = 0;

		virtual RefPtr<GraphicsDevice> GetGraphicsDevice() const = 0;
		virtual RefPtr<PhysicalGraphicsDevice> GetPhysicalGraphicsDevice() const = 0;

	private:
		inline static GraphicsContext* s_context;
		inline static GraphicsAPI s_graphicsAPI;
	};
}
