#pragma once

#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Core/RHIInterface.h"

#include "VoltRHI/Memory/Allocator.h"

namespace Volt::RHI
{
	class GraphicsDevice;
	class PhysicalGraphicsDevice;

	class GraphicsContext : public RHIInterface
	{
	public:
		GraphicsContext();
		virtual ~GraphicsContext();

		[[nodiscard]] static GraphicsContext& Get() { return *s_context; }

		[[nodiscard]] static Ref<GraphicsDevice> GetDevice() { return s_context->m_graphicsDevice; };
		[[nodiscard]] static Ref<PhysicalGraphicsDevice> GetPhysicalDevice() { return s_context->m_physicalDevice; };
		[[nodiscard]] static Allocator& GetDefaultAllocator() { return *s_context->m_defaultAllocator; };
		[[nodiscard]] static Ref<Allocator> GetTransientAllocator() { return s_context->m_transientAllocator; }

		static Ref<GraphicsContext> Create(const GraphicsContextCreateInfo& createInfo);
		
		static VT_NODISCARD GraphicsAPI GetAPI() { return s_graphicsAPI; }

		template<typename... Args>
		static void Log(Severity logSeverity, std::string_view message, Args&&... args);

		template<typename... Args>
		static void LogTagged(Severity logSeverity, std::string_view tag, std::string_view message, Args&&... args);

		static void LogUnformatted(Severity logSeverity, std::string_view message);

		static void DestroyResource(std::function<void()>&& function);

	protected:
		Ref<GraphicsDevice> m_graphicsDevice;
		Ref<PhysicalGraphicsDevice> m_physicalDevice;

		Scope<Allocator> m_defaultAllocator;
		Ref<Allocator> m_transientAllocator;

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
