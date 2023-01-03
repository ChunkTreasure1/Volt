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
	Enable();

	elements.insert({ "TestSprite", std::make_shared<UI::Sprite>("TestSprite")});
	elements.at("TestSprite")->SetPosition({ 512,0 });

	elements.at("TestSprite")->ReciveChild(std::make_shared<UI::Sprite>("TestChild"));
	Ref<UI::Element> child = elements.at("TestSprite")->GetChild("TestChild");
	child->SetPosition({ -256,0 });
	child->SetScale({ 0.5f, 0.5f });

	child->ReciveChild((std::make_shared<UI::Sprite>("TestGrandChild")));
	Ref<UI::Element> grandChild = child->GetChild("TestGrandChild");
	grandChild->SetPosition({ 0,256 });
	grandChild->SetScale({1.5f, 1.5f});

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
		elements.at("TestSprite")->SetPosition({ 500.0f,0 });
	}

	if (e.GetKeyCode() == VT_KEY_F10)
	{
		elements.at("TestSprite")->SetPosition({ -500.0f,0 });
	}

	return false;
}
