#pragma once
#include <Volt/Utility/Math.h>
#include "AnchorEnums.h"

struct UICanvas
{
	float left;
	float right;
	float bottom;
	float top;

	float width;
	float height;
	float globalScaleX = 1.0f;
	float globalScaleY = 1.0f;

	gem::vec2 TLpos = {0.f,0.f};
	gem::vec2 TRpos = {0.f,0.f};
	gem::vec2 BLpos = {0.f,0.f};
	gem::vec2 BRpos = {0.f,0.f};
	gem::vec2 Cpos = {0.f,0.f};

	UICanvas(float aWidth, float aHeight, float viewportX = 0, float viewportY = 0)
	{
		left = (-aWidth / 2.f);
		right = (aWidth / 2.f);
		bottom = (-aHeight / 2.f);
		top = (aHeight / 2.f);

		globalScaleX = aWidth / 1920.f;
		globalScaleY = aHeight / 1080.f;

		if (globalScaleX <= 0.000001f && globalScaleY <= 0.00000f) 
		{
			globalScaleX = 1.f;
			globalScaleY = 1.f;
		}
	
		width = aWidth;
		height = aHeight;

		TLpos = { viewportX, viewportY };
		TRpos = { viewportX + aWidth, viewportY };
		BLpos = { viewportX, viewportY + aHeight };
		BRpos = { viewportX + aWidth, viewportY + aHeight };
		Cpos = { ((viewportX + (aWidth / 2))), ((viewportY + (aHeight/2))) };
	}

	UICanvas(const UICanvas& aCanvas) 
	{
		left = aCanvas.left;
		right = aCanvas.right;
		bottom = aCanvas.bottom;
		top = aCanvas.top;

		width = aCanvas.width;
		height = aCanvas.height;
		globalScaleX = aCanvas.globalScaleX;
		globalScaleY = aCanvas.globalScaleY;

		TLpos = aCanvas.TLpos;
		TRpos = aCanvas.TRpos;
		BLpos = aCanvas.BLpos;
		BRpos = aCanvas.BRpos;
		Cpos = aCanvas.Cpos;
	}

};