#include "gepch.h"
#include "HUDLayerBase.h"

#include <Volt/Input/Input.h>
#include <Volt/Input/MouseButtonCodes.h>
#include <Volt/Input/KeyCodes.h>

#include <Volt/Rendering/Renderer.h>

#include <Volt/UI/Elements/Sprite.hpp>

HUD::LayerBase::LayerBase(Ref<Volt::SceneRenderer>& aSceneRenderer, const std::string& aLayerName) : renderpassRef(aSceneRenderer)
{
	layerName = aLayerName;
	canvas = std::make_shared<UI::Canvas>(1920.f, 1080.f);
	camera = CreateRef<Volt::Camera>(canvas->left, canvas->right, canvas->bottom, canvas->top, 0.001f, 10000.f);
	screenspaceShader = Volt::ShaderRegistry::Get("Quad");
}

void HUD::LayerBase::Enable()
{
	isEnabled = true;
}

void HUD::LayerBase::Disable()
{
	isEnabled = false;
}

void HUD::LayerBase::OnAttach()
{
	renderPass.framebuffer = renderpassRef->GetFinalFramebuffer();
}

void HUD::LayerBase::OnDetach()
{

}

void HUD::LayerBase::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(HUD::LayerBase::OnKeyEvent));
	//dispatcher.Dispatch<Volt::ViewportResizeEvent>(VT_BIND_EVENT_FN(UIBaseLayer::OnViewportResize));

	if (!isEnabled) { return; }

	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(HUD::LayerBase::OnUpdate));
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(HUD::LayerBase::OnRender));
}

bool HUD::LayerBase::OnRender(Volt::AppRenderEvent& e)
{
	Volt::Renderer::Begin(layerName);
	Volt::Renderer::BeginPass(renderPass, camera, false);

	Volt::Renderer::SetDepthState(Volt::DepthState::None);

	for (auto& element : elements) 
	{
		if (element.second->GetType() == UI::ElementType::SPRITE)
		{
			Ref<UI::Sprite> sprite = std::dynamic_pointer_cast<UI::Sprite>(element.second);
			sprite->Render();
		}

		for (auto& child : element.second->children) 
		{
			if (child.second->GetType() == UI::ElementType::SPRITE)
			{
				Ref<UI::Sprite> sprite = std::dynamic_pointer_cast<UI::Sprite>(child.second);
				sprite->Render();
			}
		}
	}

	Volt::Renderer::DispatchSpritesWithShader(screenspaceShader);
	Volt::Renderer::DispatchText();

	Volt::Renderer::EndPass();
	Volt::Renderer::End();

	return false;
}

bool HUD::LayerBase::OnUpdate(Volt::AppUpdateEvent& e)
{
	return false;
}

bool HUD::LayerBase::OnKeyEvent(Volt::KeyPressedEvent& e)
{
	return false;
}
