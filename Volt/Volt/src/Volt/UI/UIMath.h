#pragma once
#include <Volt/Utility/Math.h>
#include "UICanvas.h"
#include "AnchorEnums.h"

namespace UIMath
{
	//Converts from Windows Alinged Coords to "Normal" Cartesian Coords (X.right = Positive, Y.up = Positive)
	inline gem::vec2 GetConvertedCoordinates(gem::vec2 aPosition)
	{
		return { aPosition.x, -aPosition.y };
	}

	//Converts from Windows Alinged Coords to "Normal" Cartesian Coords (X.right = Positive, Y.up = Positive)
	inline gem::vec2 GetConvertedCoordinates(float x, float y)
	{
		return { x, -y };
	}

	//Converts from screenspace position down to normalized position in Viewportspace
	inline gem::vec2 GetNormalizedPosition(gem::vec2 aPosition)
	{
		return { aPosition.x / 1920, aPosition.y / 1080, };
	}

	//Converts from screenspace position down to normalized position in Viewportspace
	inline gem::vec2 GetNormalizedPosition(float x, float y)
	{
		return { x / 1920, y / 1080 };
	}

	//Expects a Normalized Value
	inline gem::vec2 GetViewportPosition(gem::vec2 aPosition, const UICanvas& aCanvas)
	{
		return { aCanvas.width * aPosition.x, aCanvas.height * aPosition.y };
	}

	//Expects Windows Alinged Coords
	inline gem::vec2 ConvertToViewSpacePos(gem::vec2 aPosition, const UICanvas& aCanvas)
	{
		return { aPosition.x - aCanvas.TLpos.x, aPosition.y - aCanvas.TLpos.y };
	}

	//Expects Windows Aligned Coords
	inline gem::vec2 ConvertPositionToCenter(gem::vec2 aPosition, const UICanvas& aCanvas)
	{
		gem::vec2 ConvertedPos = aPosition - aCanvas.Cpos;
		return GetConvertedCoordinates(ConvertedPos);
	}

	//Returns a Viewspace position in Screen position
	inline gem::vec2 GetScreenPos(const gem::vec2 aPosition, const UICanvas& aCanvas, const eAnchored aAnchor)
	{
		gem::vec2 aScreenPos{};

		switch (aAnchor)
		{
		case eAnchored::C:
			aScreenPos = { aCanvas.Cpos + aPosition };
			break;
		case eAnchored::TL:
			aScreenPos = { aCanvas.TLpos + aPosition };
			break;
		case eAnchored::TR:
			aScreenPos = { aCanvas.TRpos + aPosition };
			break;
		case eAnchored::BL:
			aScreenPos = { aCanvas.BLpos + aPosition };
			break;
		case eAnchored::BR:
			aScreenPos = { aCanvas.BRpos + aPosition };
			break;
		default:
			break;
		}
		return aScreenPos;
	}

	inline gem::vec2 GetSpriteAnchoredPos(const UICanvas& aCanvas, const eAnchored aAnchor)
	{
		gem::vec2 aScreenPos{};

		switch (aAnchor)
		{
		case eAnchored::C:
			aScreenPos = { 0,0 };
			break;
		case eAnchored::TL:
			aScreenPos = { aCanvas.left, aCanvas.top };
			break;
		case eAnchored::TR:
			aScreenPos = { aCanvas.right, aCanvas.top };
			break;
		case eAnchored::BL:
			aScreenPos = { aCanvas.left, aCanvas.bottom };
			break;
		case eAnchored::BR:
			aScreenPos = { aCanvas.right, aCanvas.bottom };
			break;
		default:
			break;
		}
		return aScreenPos;
	}
}
