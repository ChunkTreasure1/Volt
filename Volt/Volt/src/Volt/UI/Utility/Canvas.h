#pragma once
#include <GEM/gem.h>

namespace UI 
{
	struct Canvas 
	{
		float width = 0.0f;
		float height = 0.0f;

		float left = 0.0f;
		float right = 0.0f;
		float bottom = 0.0f;
		float top = 0.0f;

		float globalScaleX = 1.0f;
		float globalScaleY = 1.0f;

		Canvas(float aWidth, float aHeight, float viewportX = 0, float viewportY = 0)
		{
			width = aWidth;
			height = aHeight;

			left = (-aWidth / 2.f);
			right = (aWidth / 2.f);
			bottom = (-aHeight / 2.f);
			top = (aHeight / 2.f);

			globalScaleX = aWidth / 1920.f;
			globalScaleY = aHeight / 1080.f;

		}

		//Canvas(const UI::Canvas& aCanvas)
		//{
		//	width = aCanvas.width;
		//	height = aCanvas.height;

		//	left = aCanvas.left;
		//	right = aCanvas.right;
		//	bottom = aCanvas.bottom;
		//	top = aCanvas.top;

		//}
	};
}