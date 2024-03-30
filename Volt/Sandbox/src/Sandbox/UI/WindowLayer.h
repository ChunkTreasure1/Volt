#pragma once

#include <Volt/Core/Layer/Layer.h>

namespace Volt
{
	class Window;
	class AppBeginFrameEvent;
	class AppPresentFrameEvent;
	class AppRenderEvent;

	namespace RHI
	{
		class CommandBuffer;
	}
}

class WindowLayer : public Volt::Layer
{
public:
	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Volt::Event& e) override;
	void LayerOnEvent(Volt::Event& e);

private:
	bool OnBeginFrame(Volt::AppBeginFrameEvent& e);
	bool OnPresentFrame(Volt::AppPresentFrameEvent& e);
	bool OnRender(Volt::AppRenderEvent& e);

	Scope<Volt::Window> m_window;
	Ref<Volt::RHI::CommandBuffer> m_commandBuffer;
};
