#include "vtpch.h"
#include "AnimatedSprite.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Utility/FileSystem.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	AnimatedSprite::AnimatedSprite(const std::filesystem::path& aSpriteSheet, uint32_t aFrameCount, uint32_t aFramesPerRow, float framesPerSecond)
		: myFrameCount(aFrameCount), myPerFrameTime(1.f / framesPerSecond)
	{
		const Ref<Texture2D> texture = Volt::AssetManager::GetAsset<Volt::Texture2D>(aSpriteSheet);
		if (!texture || !texture->IsValid())
		{
			return;
		}

		const float cellSize = (float)(texture->GetWidth() / aFramesPerRow);

		float x = 0.f;
		float y = 0.f;

		for (uint32_t i = 0; i < aFrameCount; i++)
		{
			if (i == aFramesPerRow)
			{
				y += 1.f;
				x = 0.f;
			}

			auto& subTexture = mySubTextures.emplace_back();
			subTexture = SubTexture2D::CreateFromCoords(texture, { x, y }, { cellSize, cellSize });
			x++;
		}
	}

	void AnimatedSprite::Play()
	{
		myIsPlaying = true;
	}

	void AnimatedSprite::Stop()
	{
		myIsPlaying = false;
	}

	void AnimatedSprite::Render(const gem::mat4& aTransform) const
	{
		Volt::Renderer::SubmitSprite(mySubTextures.at(myCurrentFrame), aTransform);
	}

	void AnimatedSprite::Update(float aDeltaTime)
	{
		if (!myIsPlaying)
		{
			return;
		}

		myCurrentTime += aDeltaTime;
		if (myCurrentTime > myPerFrameTime)
		{
			myCurrentFrame++;
			myCurrentTime = 0.f;
			if (myCurrentFrame >= myFrameCount)
			{
				if (myShouldLoop)
				{
					myCurrentFrame = 0;
				}
				else
				{
					myCurrentFrame = myFrameCount - 1;
				}
			}
		}
	}
}