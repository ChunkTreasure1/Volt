#include "gkpch.h"
#include "AnimationDurationNode.h"

#include "GraphKey/Nodes/Animation/BaseAnimationNodes.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Animation/AnimationGraphAsset.h>
#include <Volt/Asset/Animation/AnimatedCharacter.h>

#include <Volt/Asset/AssetManager.h>

#include <Volt/Scene/SceneManager.h>

#include <Volt/Components/Components.h>

#include <Volt/Scripting/Mono/MonoScriptEngine.h>
#include <Volt/Scripting/Mono/MonoScriptInstance.h>

namespace GraphKey
{
	GetAnimationDurationNode::GetAnimationDurationNode()
	{
		inputs =
		{
			AttributeConfigDefault("", AttributeDirection::Input, Volt::AssetHandle(0)),
			AttributeConfigDefault("Speed", AttributeDirection::Input, 1.f),
			AttributeConfigDefault("Duration", AttributeDirection::Input, 1.f),
		};

		outputs =
		{
			AttributeConfig<float>("Duration", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetAnimationDurationNode::GetAnimationDuration)),
			AttributeConfig<float>("Speed", AttributeDirection::Output, true, GK_BIND_FUNCTION(GetAnimationDurationNode::GetAnimationSpeed))
		};
	}

	void GetAnimationDurationNode::GetAnimationDuration()
	{
		auto anim = GetAnimation();
		SetOutputData<float>(0, GetInput<float>(1) * ((anim) ? GetAnimation()->GetDuration() : 1.f));
	}

	void GetAnimationDurationNode::GetAnimationSpeed()
	{
		auto anim = GetAnimation();
		SetOutputData<float>(1, ((anim) ? GetAnimation()->GetDuration() : 1.f) / GetInput<float>(2));
	}

	Ref<Volt::Animation> GetAnimationDurationNode::GetAnimation()
	{
		const auto animHandle = GetInput<Volt::AssetHandle>(0);
		return Volt::AssetManager::GetAsset<Volt::Animation>(animHandle);
	}
}
