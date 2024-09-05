#pragma once

#include "Testing/RenderingTestBase.h"

#include <Volt/Core/Layer/Layer.h>
#include <Volt/Events/ApplicationEvents.h>
#include <WindowModule/Events/WindowEvents.h>

#include <EventSystem/EventListener.h>

class RenderingTestingLayer : public Volt::Layer, public Volt::EventListener
{
public:
	RenderingTestingLayer() = default;
	~RenderingTestingLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;

private:
	bool OnRenderEvent(Volt::WindowRenderEvent& e);

	Vector<Scope<RenderingTestBase>> m_renderingTests;
};
