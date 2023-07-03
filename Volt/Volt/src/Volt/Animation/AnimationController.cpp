#include "vtpch.h"
#include "AnimationController.h"

#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Components/Components.h"

#include <GraphKey/Nodes/Animation/BaseAnimationNodes.h>

namespace Volt
{
	AnimationController::AnimationController(Ref<Volt::AnimationGraphAsset> graph)
	{
		myGraph = graph->CreateCopy();
	}

	AnimationController::AnimationController(Ref<Volt::AnimationGraphAsset> graph, Entity entity)
		: myEntity(entity)
	{
		myGraph = graph->CreateCopy(entity.GetId());
	}

	const std::vector<glm::mat4> AnimationController::Sample()
	{
		VT_PROFILE_FUNCTION();

		if (!myGraph)
		{
			return {};
		}

		const auto nodes = myGraph->GetNodesOfType("OutputPoseNode");
		if (nodes.empty())
		{
			return {};
		}

		auto outputPoseNode = std::reinterpret_pointer_cast<GraphKey::OutputPoseNode>(nodes.at(0));
		const auto character = AssetManager::GetAsset<AnimatedCharacter>(myGraph->GetCharacterHandle());

		const auto sample = outputPoseNode->Sample(true, 0.f);
		const auto& invBindPose = character->GetSkeleton()->GetInverseBindPose();

		std::vector<glm::mat4> result{};
		result.resize(sample.pose.size());

		myRootMotion = sample.rootTRS;

		for (size_t i = 0; i < sample.pose.size(); i++)
		{
			const auto& trs = sample.pose.at(i);

			const glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, trs.position)* glm::mat4_cast(trs.rotation)* glm::scale(glm::mat4{ 1.f }, trs.scale);
			result[i] = transform * invBindPose[i];
		}

		{
			VT_PROFILE_SCOPE("Joint Attachments");

			for (const auto& [attachmentId, attachedEntities] : myAttachedEntities)
			{
				auto attachment = character->GetJointAttachmentFromID(attachmentId);
				if (attachment.jointIndex == -1)
				{
					continue;
				}

				if (static_cast<int32_t>(sample.pose.size()) <= attachment.jointIndex)
				{
					continue;
				}

				const auto& currentTRS = sample.pose.at(attachment.jointIndex);

				const glm::vec3 resultPos = currentTRS.position + currentTRS.rotation * attachment.positionOffset;
				const glm::quat resultRot = currentTRS.rotation * attachment.rotationOffset;

				for (auto ent : attachedEntities)
				{
					ent.SetLocalPosition(resultPos);
					ent.SetLocalRotation(resultRot);
					ent.SetLocalScale(currentTRS.scale);
				}
			}
		}

		return result;
	}

	void AnimationController::AttachEntity(const std::string& attachment, Entity entity)
	{
		const auto character = AssetManager::GetAsset<AnimatedCharacter>(myGraph->GetCharacterHandle());

		if (!character || !character->IsValid())
		{
			return;
		}

		if (!character->HasJointAttachment(attachment))
		{
			return;
		}

		myAttachedEntities[character->GetJointAttachmentFromName(attachment).id].emplace_back(entity);
	}

	void AnimationController::DetachEntity(Entity entity)
	{
		for (auto& [attachmentName, attachedEntities] : myAttachedEntities)
		{
			auto it = std::find(attachedEntities.begin(), attachedEntities.end(), entity);
			if (it != attachedEntities.end())
			{
				attachedEntities.erase(it);
				return;
			}
		}
	}

	void AnimationController::OnEvent(Event& e)
	{
		if (myGraph)
		{
			myGraph->OnEvent(e);
		}
	}
}
