#pragma once

#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Core/RHIInterface.h"

#include "VoltRHI/Memory/Allocator.h"

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
		VT_INLINE VT_NODISCARD static Ref<GraphicsDevice> GetDevice() { return s_context->GetGraphicsDevice(); };
		VT_INLINE VT_NODISCARD static Ref<PhysicalGraphicsDevice> GetPhysicalDevice() { return s_context->GetPhysicalGraphicsDevice(); };
		VT_INLINE VT_NODISCARD static Allocator& GetDefaultAllocator() { return s_context->GetDefaultAllocatorImpl(); };
		VT_INLINE VT_NODISCARD static Ref<Allocator> GetTransientAllocator() { return s_context->GetTransientAllocatorImpl(); }
		VT_INLINE VT_NODISCARD static GraphicsAPI GetAPI() { return s_graphicsAPI; }

		static Ref<GraphicsContext> Create(const GraphicsContextCreateInfo& createInfo);

		static void Update();

	protected:
		virtual Allocator& GetDefaultAllocatorImpl() = 0;
		virtual Ref<Allocator> GetTransientAllocatorImpl() = 0;

		virtual Ref<GraphicsDevice> GetGraphicsDevice() const = 0;
		virtual Ref<PhysicalGraphicsDevice> GetPhysicalGraphicsDevice() const = 0;

	private:
		inline static GraphicsContext* s_context;
		inline static GraphicsAPI s_graphicsAPI;
	};
}
