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

		template<typename... Args>
		static void Log(Severity logSeverity, std::string_view message, Args&&... args);

		template<typename... Args>
		static void LogTagged(Severity logSeverity, std::string_view tag, std::string_view message, Args&&... args);

		static void LogUnformatted(Severity logSeverity, std::string_view message);
		static void DestroyResource(std::function<void()>&& function);
		static void Update();

	protected:
		virtual Allocator& GetDefaultAllocatorImpl() = 0;
		virtual Ref<Allocator> GetTransientAllocatorImpl() = 0;

		virtual Ref<GraphicsDevice> GetGraphicsDevice() const = 0;
		virtual Ref<PhysicalGraphicsDevice> GetPhysicalGraphicsDevice() const = 0;

	private:
		inline static GraphicsContext* s_context;
		inline static GraphicsAPI s_graphicsAPI;
		inline static LogHookInfo s_logHook;
		inline static ResourceManagementInfo s_resourceManagementInfo;
	};

	template<typename ...Args>
	inline void GraphicsContext::Log(Severity logSeverity, std::string_view message, Args && ...args)
	{
		if (!s_logHook.enabled || !s_logHook.logCallback)
		{
			return;
		}

		s_logHook.logCallback(logSeverity, std::vformat(message, std::make_format_args(args...)));
	}

	template<typename... Args>
	inline void GraphicsContext::LogTagged(Severity logSeverity, std::string_view tag, std::string_view message, Args&&... args)
	{
		if (!s_logHook.enabled || !s_logHook.logCallback)
		{
			return;
		}

		s_logHook.logCallback(logSeverity, std::format("{0}: {1}", tag, std::vformat(message, std::make_format_args(args...))));
	}
}
