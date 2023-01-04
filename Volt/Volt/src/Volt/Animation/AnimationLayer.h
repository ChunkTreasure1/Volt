#pragma once

#include "Volt/Asset/Animation/Animation.h"

namespace Volt
{
	class AnimationLayer
	{
	public:
		AnimationLayer(const std::string& name);

		const std::vector<Animation::TRS> Sample() const;

	private:
		std::string myName;
	};
}