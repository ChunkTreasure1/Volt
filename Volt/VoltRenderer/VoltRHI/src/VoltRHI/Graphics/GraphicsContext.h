#pragma once
#include "VoltRHI/Core/Core.h"
#include "VoltRHI/Core/RHICommon.h"


namespace Volt
{
	class GraphicsDevice;
	class PhysicalGraphicsDevice;

	class GraphicsContext
	{
	public:
		GraphicsContext()
		{
			s_context = this;
		};

		virtual ~GraphicsContext()
		{
			s_context = nullptr;
		};
		
		static GraphicsContext& Get() { return *s_context; }

		static Ref<GraphicsDevice> GetDevice() { return s_context->m_device; };
		static Ref<PhysicalGraphicsDevice> GetPhysicalDevice() { return s_context->m_physicalDevice; };

		static Ref<GraphicsContext> Create(const GraphicsContextCreateInfo& createInfo);

		static VT_NODISCARD GraphicsAPI GetAPI() { return s_graphicsAPI; }

		static void Log(Severity logSeverity, std::string_view message);

	protected:
		Ref<GraphicsDevice> m_device;
		Ref<PhysicalGraphicsDevice> m_physicalDevice;
		
	private:
		inline static GraphicsContext* s_context;
		inline static GraphicsAPI s_graphicsAPI;
		inline static LogHookInfo s_logHook;
	};
}
