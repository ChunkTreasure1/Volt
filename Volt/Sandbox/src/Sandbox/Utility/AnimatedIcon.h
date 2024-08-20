#pragma once

#include <Volt/Core/Base.h>

#include <EventSystem/EventListener.h>

#include <filesystem>

namespace Volt
{
	class Texture2D;
	class Event;
	class AppUpdateEvent;
}

class AnimatedIcon : public Volt::EventListener
{
public:
	AnimatedIcon(const std::filesystem::path& firstFrame, uint32_t frameCount, float animTime = 1.f);

	VT_NODISCARD VT_INLINE Ref<Volt::Texture2D> GetCurrentFrame() const { return m_currentTexture; }

	VT_INLINE void Play() { m_isPlaying = true; }
	VT_INLINE void Stop() { m_isPlaying = false; m_currentTexture = m_textures.at(0); }

	VT_INLINE void SetIsEnabled(bool state) { m_isEnabled = state; }

private:
	bool Animate(Volt::AppUpdateEvent& e);

	Vector<Ref<Volt::Texture2D>> m_textures;
	Ref<Volt::Texture2D> m_currentTexture;

	float m_animationTime = 0.f;
	float m_perFrameTime = 0.f;
	uint32_t m_frameCount = 0;

	float m_currentTime = 0.f;
	uint32_t m_currentFrame = 0;

	bool m_isPlaying = false;
	bool m_isEnabled = false;
};
