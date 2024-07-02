#pragma once

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
};
