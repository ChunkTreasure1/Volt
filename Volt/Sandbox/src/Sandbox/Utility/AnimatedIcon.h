#pragma once

#include <Volt/Core/Base.h>
#include <Volt/Events/Event.h>
#include <Volt/Events/ApplicationEvent.h>

#include <filesystem>

namespace Volt
{
	class Texture2D;
}

class AnimatedIcon
{
public:
	AnimatedIcon(const std::filesystem::path& firstFrame, uint32_t frameCount, float animTime = 1.f);

	void OnEvent(Volt::Event& e);
	inline Ref<Volt::Texture2D> GetCurrentFrame() const { return myCurrentTexture; }

	inline void Play() { myIsPlaying = true; }
	inline void Stop() { myIsPlaying = false; myCurrentTexture = myTextures.at(0); }

private:
	bool Animate(Volt::AppUpdateEvent& e);

	std::vector<Ref<Volt::Texture2D>> myTextures;
	Ref<Volt::Texture2D> myCurrentTexture;

	float myAnimationTime = 0.f;
	float myPerFrameTime = 0.f;
	uint32_t myFrameCount = 0;

	float myCurrentTime = 0.f;
	uint32_t myCurrentFrame = 0;

	bool myIsPlaying = false;
};