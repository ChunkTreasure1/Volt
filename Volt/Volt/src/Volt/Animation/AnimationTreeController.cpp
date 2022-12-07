#include "vtpch.h"
#include "AnimationTreeController.h"
#include "Volt/Asset/AssetManager.h"
#include <yaml-cpp/yaml.h>

#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "AnimationManager.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Components/Components.h"

Volt::AnimationTreeController::AnimationTreeController(std::string aAnimTree)
{
	auto treePath = "Assets/Animations/" + aAnimTree + ".animtree";

	std::ifstream file(treePath);
	std::stringstream sstream;
	sstream << file.rdbuf();

	YAML::Node root = YAML::Load(sstream.str());

	myAnimatedCharacter = root["CHRPath"].as<uint64_t>();
	YAML::Node Nodes = root["Nodes"];

	const size_t nodesize = Nodes.size();
	myNodes.resize(nodesize);

	if (Nodes)
	{
		for (size_t currentNode = 0; currentNode < nodesize; currentNode++)
		{
			std::string aName = Nodes[currentNode]["Name"].as<std::string>();
			int nodeID = Nodes[currentNode]["NodeID"].as<int>();
			int animationIndex = Nodes[currentNode]["AnimationIndex"].as<int>();

			//AnimationNode node(aName, nodeID, animationIndex);
			myNodes[currentNode] = CreateRef<AnimationNode>(aName, nodeID, animationIndex);
		}

		for (size_t currentNode = 0; currentNode < nodesize; currentNode++)
		{
			std::vector<int> NodeLinks = Nodes[currentNode]["NodePaths"].as<std::vector<int>>();

			for (auto nodelink : NodeLinks)
			{
				AnimationLink aLink;
				aLink.ToNode = GetNodeFromNodeID(nodelink);

				//Get Blend requirements
				myNodes[currentNode]->AddLink(aLink);
			}
		}

		myCurrentNode = myNodes[0].get();
		myCurrentNode->currentStartTime = AnimationManager::globalClock;
	}
	
	YAML::Node Links = root["Links"];
	if (Links)
	{

	}

	YAML::Node Parameters = root["Parameters"];
	if (Parameters)
	{
	}

}

Volt::AnimationTreeController::~AnimationTreeController()
{

}

void Volt::AnimationTreeController::Update()
{
	if (myAnimatedCharacter != Asset::Null())
	{
		auto animChar = AssetManager::GetAsset<AnimatedCharacter>(myAnimatedCharacter); // Fetch once
		if (animChar && animChar->IsValid())
		{
			if (myCurrentNode->isCrossFading)
			{
				if (AnimationManager::globalClock > myCurrentNode->crossfadeStartTime + animChar->GetCrossfadeTimeFromAnimations(myCurrentNode->crossfadeFrom, myCurrentNode->crossfadeTo))
				{
					myCurrentNode->isCrossFading = false;
					myCurrentNode->shouldCrossfade = false;
					myCurrentNode->currentStartTime = AnimationManager::globalClock;
				}
			}
			else
			{
				if (myCurrentNode->shouldCrossfade)
				{
					myCurrentNode->isCrossFading = true;
					myCurrentNode->crossfadeStartTime = AnimationManager::globalClock;

				}
				else
				{

					//Check if it is looping and should restart the loop
					if (myCurrentNode->isLooping && myCurrentNode->currentStartTime + animChar->GetAnimationDuration(myCurrentNode->currentAnimation) <= AnimationManager::globalClock)
					{
						myCurrentNode->currentStartTime = AnimationManager::globalClock;
					}
					else if(!myCurrentNode->isLooping && myCurrentNode->currentStartTime + animChar->GetAnimationDuration(myCurrentNode->currentAnimation) <= AnimationManager::globalClock)
					{
						//Move to new node
						myCurrentNode = myCurrentNode->myLinks[0].ToNode;
						myCurrentNode->currentStartTime = AnimationManager::globalClock;
					}

				}
			}
		}
	}

}

void Volt::AnimationTreeController::Render(const Wire::EntityId id, const Volt::TransformComponent& transformComp, const Volt::EntityDataComponent& dataComp, const Ref<Scene> aScene)
{
	if (myAnimatedCharacter != Asset::Null() && transformComp.visible)
	{
		auto animChar = AssetManager::GetAsset<AnimatedCharacter>(myAnimatedCharacter);
		if (animChar && animChar->IsValid())
		{
			const gem::mat4 transform = aScene->GetWorldSpaceTransform(Entity(id, aScene.get()));

			if (myCurrentNode->isCrossFading)
			{
				//Renderer::Submit(animChar->GetSkin(), transform, animChar->SampleCrossfadingAnimation(myCurrentNode->crossfadeFrom, myCurrentNode->crossfadeTo, myCurrentNode->crossfadeStartTime), id, dataComp.timeSinceCreation, myCurrentNode->castShadows);
			}
			else
			{
				Renderer::Submit(animChar->GetSkin(), transform, animChar->SampleAnimation(myCurrentNode->currentAnimation, myCurrentNode->currentStartTime), id, dataComp.timeSinceCreation, myCurrentNode->castShadows);
			}

		}
	}
}

Volt::AnimationNode* Volt::AnimationTreeController::GetNodeWithAnimation(int animationIndex)
{
	for (auto& node : myNodes)
	{
		if (node->GetAnimationIndex() == animationIndex)
		{
			return node.get();
		}
	}
	return nullptr;
}

Volt::AnimationNode* Volt::AnimationTreeController::GetNodeFromNodeID(int aNodeID)
{
	for (auto& node : myNodes)
	{
		if (node->GetNodeID() == aNodeID)
		{
			return node.get();
		}
	}
	return nullptr;
}
