#pragma once
#include <Volt/Core/Layer/Layer.h>

#include <Volt/Events/Event.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Events/MouseEvent.h>
#include <Volt/Events/KeyEvent.h>

#include <Volt/UI/Utility/Canvas.h>
#include <Volt/UI/Elements/ElementBase.hpp>

#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/Camera/Camera.h>
#include <Volt/Rendering/SceneRenderer.h>


#include <unordered_map>

namespace Volt
{
	class SceneRenderer;
}

namespace HUD
{
	class LayerBase : public Volt::Layer
	{
	public:
		LayerBase(Ref<Volt::SceneRenderer>& aSceneRenderer, const std::string& aLayerName);
		virtual ~LayerBase() = default;

		virtual void Enable();
		virtual void Disable();

		bool GetIsEnabled() { return isEnabled; }

	protected:
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnEvent(Volt::Event& e) override;

		virtual bool OnRender(Volt::AppRenderEvent& e);
		virtual bool OnUpdate(Volt::AppUpdateEvent& e);

		virtual bool OnPlay(Volt::OnScenePlayEvent& e);
		virtual bool OnStop(Volt::OnSceneStopEvent& e);
		bool OnSceneLoaded(Volt::OnSceneLoadedEvent& e);
		virtual bool OnKeyEvent(Volt::KeyPressedEvent& e);
		virtual bool OnMouseEvent(Volt::MouseButtonPressedEvent& e);
		virtual bool OnViewportResize(Volt::ViewportResizeEvent& e);



	private:
		Ref<Volt::SceneRenderer>& renderpassRef;
		Ref<UI::Canvas> canvas;

		bool isEnabled = false;

		Ref<Volt::Camera> camera;
		Ref<Volt::Shader> screenspaceShader;
		Volt::RenderPass renderPass;

		std::string settingPath;
		std::string layerName;

		std::unordered_map<std::string, UI::Element> elements;

	};
}

