#pragma once

#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Core/RHIInterface.h"

#include "VoltRHI/Memory/Allocator.h"
#include "VoltRHI/Graphics/GraphicsDevice.h"
#include "VoltRHI/Graphics/PhysicalGraphicsDevice.h"

namespace Volt::RHI
{
	class GraphicsDevice;
	class PhysicalGraphicsDevice;

	class VTRHI_API GraphicsContext : public RHIInterface
	{
	public:
		GraphicsContext();
		virtual ~GraphicsContext();

		VT_INLINE VT_NODISCARD static GraphicsContext& Get() { return *s_context; }
		VT_INLINE VT_NODISCARD static RefPtr<GraphicsDevice> GetDevice() { return s_context->GetGraphicsDevice(); };
		VT_INLINE VT_NODISCARD static RefPtr<PhysicalGraphicsDevice> GetPhysicalDevice() { return s_context->GetPhysicalGraphicsDevice(); };
		VT_INLINE VT_NODISCARD static RefPtr<Allocator> GetDefaultAllocator() { return s_context->GetDefaultAllocatorImpl(); };
		VT_INLINE VT_NODISCARD static RefPtr<Allocator> GetTransientAllocator() { return s_context->GetTransientAllocatorImpl(); }
		VT_INLINE VT_NODISCARD static GraphicsAPI GetAPI() { return s_graphicsAPI; }

		static RefPtr<GraphicsContext> Create(const GraphicsContextCreateInfo& createInfo);

		static void Update();

	protected:
		virtual RefPtr<Allocator> GetDefaultAllocatorImpl() = 0;
		virtual RefPtr<Allocator> GetTransientAllocatorImpl() = 0;

		virtual RefPtr<GraphicsDevice> GetGraphicsDevice() const = 0;
		virtual RefPtr<PhysicalGraphicsDevice> GetPhysicalGraphicsDevice() const = 0;

	private:
		inline static GraphicsContext* s_context;
		inline static GraphicsAPI s_graphicsAPI;
	};
}
