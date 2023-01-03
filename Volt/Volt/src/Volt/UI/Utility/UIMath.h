#pragma once
#include <Volt/UI/Utility/Canvas.h>

namespace UIMath 
{
	//Converts from screenspace position down to normalized position in Viewportspace
	inline gem::vec2 GetNormalizedPosition(gem::vec2 aPosition)
	{
		return { aPosition.x / 1920, aPosition.y / 1080, };
	}

	//Expects a Normalized Value
	inline gem::vec2 GetViewportPosition(gem::vec2 aPosition, const UI::Canvas& aCanvas)
	{
		return { aCanvas.width * aPosition.x, aCanvas.height * aPosition.y };
	}
}
