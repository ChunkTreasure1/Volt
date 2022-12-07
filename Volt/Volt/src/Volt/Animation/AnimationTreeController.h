#pragma once
#include "AnimationNode.h"
#include "Wire/Entity.h"
#include "Volt/Asset/Asset.h"
#include <unordered_map>

namespace Volt
{
	struct TransformComponent;
	struct EntityDataComponent;
	class Scene;
	class AnimationTreeController
	{
	public:
		AnimationTreeController(std::string aAnimTree);
		~AnimationTreeController();
		
		void Update();
		void Render(const Wire::EntityId id, const TransformComponent& transformComp, const EntityDataComponent& dataComp, const Ref<Scene> aScene);

		Volt::AnimationNode* GetNodeWithAnimation(int animationIndex);
		Volt::AnimationNode* GetNodeFromNodeID(int aNodeID);

		bool CanGoToNextNode(AnimationNode*& outNode);

	private:
		AssetHandle myHandle;
		AssetHandle myAnimatedCharacter = Asset::Null();

		AnimationNode* myCurrentNode = nullptr;
		std::vector<Ref<AnimationNode>> myNodes;

		std::unordered_map<std::string, float> myParameters;

	};
}