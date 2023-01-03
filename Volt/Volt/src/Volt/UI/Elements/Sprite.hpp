#pragma once
#include <Volt/UI/Elements/ElementBase.hpp>
#include "Volt/Rendering/Texture/Texture2D.h"
#include <Volt/Asset/AssetManager.h>
#include <Volt/Rendering/Renderer.h>

namespace UI
{
	struct SpriteData 
	{
		Ref<Volt::Texture2D> texture;
		gem::vec4 color = gem::vec4{ 1.0f,1.0f,1.0f,1.0f };
	};

	class Sprite : public Element
	{
	public:
		Sprite(const std::string aName) : Element(aName)
		{
			type = UI::ElementType::SPRITE;
			data.texture = Volt::AssetManager::GetAsset<Volt::Texture2D>("Assets/UI/Default/spriteTest.dds");
			SetTransform(gem::mat4{ 1.0f });
			SetSize({ (float)data.texture->GetWidth(), (float)data.texture->GetHeight() });
			SetColor({ 1.0f,1.0f,1.0f,1.0f });
		}
		~Sprite() override = default;

		void Render()
		{
			gem::mat4 spriteTransform = GetWorldPosition();
			gem::vec2 worldScale = GetWorldScale();

			//check if is in Sandbox or relese!
			gem::vec2 VP_position = UIMath::GetViewportPosition({ spriteTransform[3][0],spriteTransform[3][1] }, *canvas);

			spriteTransform[3][0] = VP_position.x;
			spriteTransform[3][1] = VP_position.y;

			gem::mat4 scaledTransform = spriteTransform * gem::scale(gem::mat4(1.0f), gem::vec3{ GetSize().x * worldScale.x * canvas->globalScaleX, GetSize().y * worldScale.y * canvas->globalScaleY, 1});

			Volt::Renderer::SubmitSprite(GetTexture(), scaledTransform, 0, GetColor());

			for (auto& child : children)
			{
				if (child.second->GetType() == UI::ElementType::SPRITE)
				{
					Ref<UI::Sprite> sprite = std::dynamic_pointer_cast<UI::Sprite>(child.second);
					sprite->Render();
				}
			}
		}

		std::shared_ptr<Volt::Texture2D> GetTexture() { return data.texture; }
		void SetTexture(Ref<Volt::Texture2D> aTexture) { data.texture = aTexture; }
		void SetTexture(std::string aPath) { data.texture = Volt::AssetManager::GetAsset<Volt::Texture2D>(aPath); }

		gem::vec4& GetColor() { return data.color; }
		void SetColor(gem::vec4 aColor) { data.color = aColor; }

		float GetAlpha() { return data.color[3]; }
		void SetAlpha(float aAlpha) { data.color[3] = aAlpha; }

	private:
		SpriteData data;
	};
}