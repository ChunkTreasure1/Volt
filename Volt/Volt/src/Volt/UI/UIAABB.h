#pragma once
#include <Volt/Utility/Math.h>
#include "UIElement.h"

class UIAABB
{
	//FUNCTIONS
public:
	UIAABB()
	{

	}

	bool IsInside(gem::vec2 p, UIElement& aUIElement)
	{
		if (!isEnabled) { return false; }

		gem::vec2 position = aUIElement.GetPosition();
		gem::vec2 pivot = aUIElement.GetPivot();
		gem::vec2 size = aUIElement.GetSize();
		float globalXScale = aUIElement.GetCanvas()->globalScaleX;
		float globalYScale = aUIElement.GetCanvas()->globalScaleY;

		return p.x >= position.x - size.x * globalXScale
			&& p.x < position.x + size.x * globalXScale
			&& p.y >= position.y - size.y * globalYScale
			&& p.y < position.y + size.y * globalYScale;
	}

private:

	//VARIABLES
public:
	bool isEnabled = true;

private:

};
