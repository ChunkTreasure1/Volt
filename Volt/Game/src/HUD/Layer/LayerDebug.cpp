#include "gepch.h"
#include "LayerDebug.h"

#include <Volt/Core/Base.h>

#include <Volt/Input/KeyCodes.h>

HUD::LayerDebug::LayerDebug(Ref<Volt::SceneRenderer>& aSceneRenderer, std::string aLayerName) : HUD::LayerBase(aSceneRenderer, aLayerName)
{
	VT_ASSERT(!myInstance, "DebugLayer already exists");
	instance = this;

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
	return false;
}
