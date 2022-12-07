#pragma once
#include "UIElement.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include <Volt/Asset/AssetManager.h>
#include <Volt/Rendering/Renderer.h>


class UISprite : public UIElement
{
//FUNCTIONS
public:
	UISprite() = default;

	UISprite(const char* aSpritePath)
	{
		myUIType = eUIElementType::SPRITE;
		myTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>(aSpritePath);
		SetTransform(gem::mat4{ 1.0f });
		SetSize({ (float)myTexture->GetWidth(), (float)myTexture->GetHeight() });
		myColor = gem::vec4{ 1.0f,1.0f,1.0f,1.0f };
	}

	UISprite(const char* aSpritePath, gem::mat4 aTransform, gem::vec4 aColor)
	{
		myUIType = eUIElementType::SPRITE;
		myTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>(aSpritePath);
		SetTransform(aTransform);
		SetSize({ (float)myTexture->GetWidth(), (float)myTexture->GetHeight() });
		myColor = aColor;
	}
	~UISprite() = default;

	void OnRender()
	{
		if (isEnabled)
		{
			Volt::Renderer::SubmitSprite(GetTexture(), GetTransform(), 0, GetColor());
		}
	}


	std::shared_ptr<Volt::Texture2D> GetTexture() { return myTexture; }
	void SetTexture(Ref<Volt::Texture2D> aTexture) { myTexture = aTexture; }
	void SetTexture(std::string aPath){ myTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>(aPath); }

	gem::vec4& GetColor() { return myColor; }
	void SetColor(gem::vec4 aColor) { myColor = aColor; }

	float GetAlpha() { return myColor[3]; }
	void SetAlpha(float aAlpha) { myColor[3] = aAlpha; }

private:

//VARIABLES
public:
	//bool isEnabled = true;

private:
	std::shared_ptr<Volt::Texture2D> myTexture;
	gem::vec4 myColor = gem::vec4{ 1.0f,1.0f,1.0f,1.0f };

};