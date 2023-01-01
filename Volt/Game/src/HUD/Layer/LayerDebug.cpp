#include "gepch.h"
#include "LayerDebug.h"

#include <Volt/Core/Base.h>

#include <Volt/UI/Elements/Sprite.hpp>

#include <Volt/Input/KeyCodes.h>
#include <memory>

HUD::LayerDebug::LayerDebug(Ref<Volt::SceneRenderer>& aSceneRenderer, std::string aLayerName) : HUD::LayerBase(aSceneRenderer, aLayerName)
{
	VT_ASSERT(!myInstance, "DebugLayer already exists");
	instance = this;

	elements.insert({ "TestSprite", std::make_shared<UI::Sprite>("TestSprite")});
	Enable();

	elements.at("TestSprite")->ReciveChild(std::make_shared<UI::Sprite>("TestChild"));
	Ref<UI::Element> child = elements.at("TestSprite")->GetChild("TestChild");
	child->SetPosition({ 0,0 });
	elements.at("TestSprite")->SetCanvas(canvas);


}

HUD::LayerDebug::~LayerDebug()
{
	instance = nullptr;
}

bool HUD::LayerDebug::OnKeyEvent(Volt::KeyPressedEvent& e)
{
	if (e.GetKeyCode() == VT_KEY_F8)
	{
		if (GetIsEnabled()) { Disable(); }
		else { Enable(); }
	}

	if (e.GetKeyCode() == VT_KEY_F9)
	{
		elements.at("TestSprite")->SetPosition({ 1000.0f,0 });
	}

	return false;
}
