#include "sbpch.h"
#include "AnimatedIcon.h"

#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Asset/Importers/TextureImporter.h>

AnimatedIcon::AnimatedIcon(const std::filesystem::path& firstFrame, uint32_t frameCount, float animTime)
	: myAnimationTime(animTime), myFrameCount(frameCount), myPerFrameTime(animTime / (float)frameCount)
{
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
		myTextures.emplace_back(Volt::TextureImporter::ImportTexture(path));
	}

	VT_CORE_ASSERT(!myTextures.empty(), "No frames found!");
	VT_CORE_ASSERT(myTextures.size() == frameCount, "Not all frames loaded!");
	myCurrentTexture = myTextures[0];
}

void AnimatedIcon::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(AnimatedIcon::Animate));
}

bool AnimatedIcon::Animate(Volt::AppUpdateEvent& e)
{
	if (!myIsPlaying)
	{
		return false;
	}

	myCurrentTime += e.GetTimestep();
	if (myCurrentTime > myPerFrameTime)
	{
		myCurrentFrame++;
		myCurrentTime = 0.f;
		if (myCurrentFrame >= myFrameCount)
		{
			myCurrentFrame = 0;
		}

		myCurrentTexture = myTextures.at(myCurrentFrame);
	}

	return false;
}
