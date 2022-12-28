#pragma once
#include "UIElement.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include "UISprite.h"
#include "UIText.h"
#include "UIAABB.h"

#include <Volt/Rendering/Renderer.h>
#include <iostream>

class UIPopUp : public UIElement
{
//FUNCTIONS
public:
	UIPopUp()
	{
		myUIType = eUIElementType::POPUP;
		isEnabled = false;
	}

	~UIPopUp() = default;

	void OnRender()
	{
		if (!isEnabled) { return; }

		for (auto sprite : mySprites)
		{
			//gem::vec2 newSpritePos = { /*myLastMousePos.x +*/ sprite.GetPosition().x, /*myLastMousePos.y +*/ sprite.GetPosition().y };
			gem::vec2 newSpritePos = { GetPosition().x + sprite.GetPosition().x, GetPosition().y + sprite.GetPosition().y };
			gem::mat4 newSpriteTransform = { 1.f };
			newSpriteTransform[3][0] = newSpritePos.x;
			newSpriteTransform[3][1] = newSpritePos.y;

			newSpriteTransform *= gem::scale(gem::mat4(1.0f), gem::vec3{ sprite.GetSize().x * GetScale().x * GetCanvas()->globalScaleX, sprite.GetSize().y * GetScale().y * GetCanvas()->globalScaleY, 1 });

			Volt::Renderer::SubmitSprite(sprite.GetTexture(), newSpriteTransform, 0, sprite.GetColor());
		}

		for (auto text : myTexts)
		{
			//gem::vec2 newTextPos = { myLastMousePos.x + text.GetPosition().x, myLastMousePos.y + text.GetPosition().y };
			gem::vec2 newTextPos = { GetPosition().x + text.GetPosition().x, GetPosition().y + text.GetPosition().y };
			gem::mat4 newTextTransform = { 1.f };
			newTextTransform[3][0] = newTextPos.x;
			newTextTransform[3][1] = newTextPos.y;

			newTextTransform *= gem::scale(gem::mat4(1.0f), gem::vec3{ text.GetSize().x * GetScale().x * GetCanvas()->globalScaleX, text.GetSize().y * GetScale().y * GetCanvas()->globalScaleY, 1 });

			Volt::Renderer::SubmitText(text.GetText(), text.GetFont(), newTextTransform, text.GetMaxWidth(), text.GetColor());
		}
	}

	void OnUpdate(gem::vec2 mousePos)
	{
		if (IsInside(mousePos))
		{
			myLastMousePos = mousePos;
			isEnabled = true;
		}
		else
		{
			isEnabled = false;
		}
	}

	bool IsInside(gem::vec2 mousePos)
	{
		return myCollider.IsInside(mousePos, *this);
	}

	void SetSprite(UISprite& aSprite)
	{
		mySprites.push_back(aSprite);
	}

	void SetText(UIText& aText)
	{
		myTexts.push_back(aText);
	}

	std::vector<UIText>& GetTexts()
	{
		return myTexts;
	}

private:

//VARIABLES
public:
	int myID = -1;

private:
	std::vector<UISprite> mySprites;
	std::vector<UIText> myTexts;

	UIAABB myCollider;

	gem::vec2 myLastMousePos;
};