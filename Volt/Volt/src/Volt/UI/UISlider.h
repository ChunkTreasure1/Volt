#pragma once
#include <Volt/Utility/Math.h>
#include <Volt/Rendering/Renderer.h>
#include "Volt/UI/UISprite.h"
#include "Volt/UI/UIAABB.h"
#include "Volt/UI/UIText.h"

#include <iostream>

class UISlider : public UIElement 
{
//FUNCTIONS
public:
	UISlider()
	{
		myUIType = eUIElementType::SLIDER;
		SetSize({ 64, 64 });
	}
	~UISlider() = default;

	void OnRender() 
	{
		myButtonSprite.OnRender();
		myDebugText.OnRender();
	}

	void OnUpdate(gem::vec2 mousePos, bool isMouseClicked) 
	{
		if (myAABB.IsInside(mousePos, *this)) 
		{
			if (isMouseClicked) 
			{
				//DO STUFF!
			}
		}
		else 
		{

		}
	}

private:

//VARIABLES
public:
	//bool isEnabled = true;
	UISprite myButtonSprite;
	UIText myDebugText;

	float myLength;
	float myValue;

private:

	UIAABB myAABB;


};

