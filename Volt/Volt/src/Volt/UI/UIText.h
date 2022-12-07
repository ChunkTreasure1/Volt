#pragma once
#include "UIElement.h"
#include <string>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Text/Font.h>

class UIText :public  UIElement
{
	enum TextAlign 
	{
		LEFT,
		CENTER
	};


public:
	UIText()
	{
		myUIType = eUIElementType::TEXT;
		myColor = gem::vec4{ 1.0f,1.0f,1.0f,1.0f };
	}

	UIText(std::string aText)
	{
		myUIType = eUIElementType::TEXT;
		myFont = Volt::AssetManager::GetAsset<Volt::Font>("Assets/UI/Font/diablo.ttf");
		myMaxWidth = 512;
		myColor = gem::vec4{ 1.0f,1.0f,1.0f,1.0f };
		myText = aText;
	}

	void OnRender()
	{
		if (myTextAlign == CENTER)
		{
			float aWidth = myFont->GetWidthOfString(myText, GetSize().x);

			aWidth *= 0.5f;
			aWidth *= myCanvas->globalScaleX;

			gem::vec2 newTextPos = { GetPosition().x, GetPosition().y };
			gem::mat4 newTextTransform = { 1.f };
			newTextPos.x -= aWidth;

			newTextTransform[3][0] = newTextPos.x + 8;
			newTextTransform[3][1] = newTextPos.y;

			SetTransform(newTextTransform);
		}

		Volt::Renderer::SubmitString(myText, myFont, GetTransform(), myMaxWidth, myColor);
	}

	std::string& GetText() { return myText; }
	void SetText(std::string aString) { myText = aString; }

	Ref<Volt::Font> GetFont() { return myFont; }
	void SetFont(std::string aPath) { myFont = Volt::AssetManager::GetAsset<Volt::Font>(aPath.c_str()); }

	gem::vec4& GetColor() { return myColor; }
	void SetColor(gem::vec4 aColor) { myColor = aColor; }

	void SetTextWidth(float aWidth) { myMaxWidth = aWidth; }
	float GetMaxWidth() { return myMaxWidth; }

	void SetAlignment(UINT aEnum) {
		myTextAlign = (TextAlign)aEnum;
	}

private:



public:


private:
	std::string myText;
	Ref<Volt::Font> myFont;
	float myMaxWidth = 0.f;
	gem::vec4 myColor = { 1.f };
	TextAlign myTextAlign = TextAlign::LEFT;
};