#include "sbpch.h"
#include "AnimatedIcon.h"

#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Asset/Importers/TextureImporter.h>

#include <Volt/Events/ApplicationEvents.h>

#include <EventSystem/Event.h>

AnimatedIcon::AnimatedIcon(const std::filesystem::path& firstFrame, uint32_t frameCount, float animTime)
	: m_animationTime(animTime), m_frameCount(frameCount), m_perFrameTime(animTime / (float)frameCount)
{
	RegisterListener<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(AnimatedIcon::Animate), [this]() { return m_isEnabled; });

	std::string filename = firstFrame.stem().string();
	const size_t numPos = filename.find_first_of("0123456789");
	if (numPos != std::string::npos)
	{
		filename = filename.substr(0, numPos);
	}

	const std::filesystem::path dirPath = firstFrame.parent_path();

	for (uint32_t frame = 1; frame <= frameCount; frame++)
	{
		const std::filesystem::path path = dirPath / (filename + std::to_string(frame) + firstFrame.extension().string());
		
		m_textures.emplace_back() = CreateRef<Volt::Texture2D>();
		Volt::TextureImporter::ImportTexture(path, *m_textures.back());
	}

	VT_ASSERT_MSG(!m_textures.empty(), "No frames found!");
	VT_ASSERT_MSG(m_textures.size() == frameCount, "Not all frames loaded!");
	m_currentTexture = m_textures[0];
}

bool AnimatedIcon::Animate(Volt::AppUpdateEvent& e)
{
	if (!m_isPlaying)
	{
		return false;
	}

	m_currentTime += e.GetTimestep();
	if (m_currentTime > m_perFrameTime)
	{
		m_currentFrame++;
		m_currentTime = 0.f;
		if (m_currentFrame >= m_frameCount)
		{
			m_currentFrame = 0;
		}

		m_currentTexture = m_textures.at(m_currentFrame);
	}

	return false;
}
