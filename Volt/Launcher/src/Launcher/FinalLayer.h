#pragma once

#include <Volt/Core/Layer/Layer.h>
#include <Volt/Events/ApplicationEvent.h>

namespace Volt
{
	class SceneRenderer;
	class Shader;
}

class FinalLayer : public Volt::Layer
{
public:
	FinalLayer(Ref<Volt::SceneRenderer>& aSceneRenderer);
	~FinalLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Volt::Event& e) override;

private:
	bool OnRenderEvent(Volt::AppRenderEvent& e);

	bool myDebugTargets = false;
	uint32_t myPassIndex = 0;
	uint32_t myTargetIndex = 0;

	Ref<Volt::SceneRenderer>& mySceneRenderer;
	Ref<Volt::Shader> myCopyToScreenShader;
};