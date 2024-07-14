#pragma once

#include "Testing/RenderingTestBase.h"

#include <Volt/Core/Layer/Layer.h>
#include <Volt/Events/ApplicationEvent.h>

class RenderingTestingLayer : public Volt::Layer
{
public:
	RenderingTestingLayer() = default;
	~RenderingTestingLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Volt::Event& e) override;

private:
	bool OnRenderEvent(Volt::AppRenderEvent& e);

	std::vector<Scope<RenderingTestBase>> m_renderingTests;
};
