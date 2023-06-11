#pragma once

#include "GraphKey/Node.h"
#include <Amp/AudioManager/AudioManager.h>
#include <Volt/Components/AudioComponents.h>

namespace GraphKey
{
	class PlayAudioNode : public Node
	{
	public:
		PlayAudioNode();

		const std::string GetName() override; 
		const glm::vec4 GetColor() override;

	private:
		void Play();
	};

}
