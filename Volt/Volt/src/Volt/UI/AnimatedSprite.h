#pragma once

#include "Volt/Rendering/Texture/SubTexture2D.h"

#include <filesystem>

namespace Volt
{
	class AnimatedSprite
	{
	public:
		AnimatedSprite(const std::filesystem::path& aSpriteSheet, uint32_t aFrameCount, uint32_t aFramesPerRow, float framesPerSecond = 30.f);

		void Play();
		void Stop();

		void Render(const gem::mat4& aTransform) const;
		void Update(float aDeltaTime);

		inline void SetLooping(bool aShouldLoop) { myShouldLoop = aShouldLoop; }
		inline const bool IsPlaying() const { return myIsPlaying; }

	private:
		const uint32_t myFrameCount;
		const float myPerFrameTime = 0.f;
		
		uint32_t myCurrentFrame = 0;
		float myCurrentTime = 0.f;

		bool myIsPlaying = false;
		bool myShouldLoop = true;

		std::vector<SubTexture2D> mySubTextures;
	};
}