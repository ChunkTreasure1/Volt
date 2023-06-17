#include "gkpch.h"

#include "GraphKey/Registry.h"

#include "GraphKey/Nodes/BaseNodes.h"

#include "GraphKey/Nodes/InputNodes.h"
#include "GraphKey/Nodes/PrintNodes.h"
#include "GraphKey/Nodes/UtilityNodes.h"

#include "GraphKey/Nodes/Entity/TransformNodes.h"
#include "GraphKey/Nodes/Entity/BaseEntityNodes.h"

#include "GraphKey/Nodes/Animation/BaseAnimationNodes.h"
#include "GraphKey/Nodes/Animation/BlendNodes.h"
#include "GraphKey/Nodes/Animation/SequenceNodes.h"
#include "GraphKey/Nodes/Animation/StateMachineNodes.h"
#include "GraphKey/Nodes/Animation/TransitionNodes.h"
#include "GraphKey/Nodes/Animation/AnimationSwitchNode.h"
#include "GraphKey/Nodes/Animation/AnimationDurationNode.h"

#include "GraphKey/Nodes/CustomEventNode.h"

#include "GraphKey/Nodes/ParameterNodes.h"
#include "GraphKey/Nodes/PhysicsNodes.h"

#include "GraphKey/Nodes/AudioNodes.h"
#include "GraphKey/Nodes/TimeNodes.h"
#include "GraphKey/Nodes/ComponentNodes.h"

#include "GraphKey/Nodes/Material/BaseMaterialNodes.h"

#include <Volt/Utility/UIUtility.h>

#include <GEM/gem.h>

namespace GraphKey
{
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeFloat, "Utility", GraphType::Scripting, (ToStringNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt, "Utility", GraphType::Scripting, (ToStringNode<int32_t>));

	GK_REGISTER_NODE_ANON(CallCustomEventNode, "Event", GraphType::Scripting);
	GK_REGISTER_NODE_ANON(RecieveCustomEventNode, "Event", GraphType::Scripting);

	GK_REGISTER_NODE(PrintNode, "Utility", GraphType::Scripting);

	GK_REGISTER_NODE(KeyPressedNode, "Input", GraphType::Scripting);
	GK_REGISTER_NODE(KeyReleasedNode, "Input", GraphType::Scripting);

	GK_REGISTER_NODE(SetEntityTransformNode, "Entity", GraphType::Scripting);
	GK_REGISTER_NODE(GetEntityTransformNode, "Entity", GraphType::Scripting);
	GK_REGISTER_NODE(SetEntityPositionNode, "Entity", GraphType::Scripting);
	GK_REGISTER_NODE(SetEntityRotationNode, "Entity", GraphType::Scripting);
	GK_REGISTER_NODE(AddEntityRotationNode, "Entity", GraphType::Scripting);

	GK_REGISTER_NODE(CreateEntityNode, "Entity", GraphType::Scripting);
	GK_REGISTER_NODE(EntityNode, "Entity", GraphType::Scripting);
	GK_REGISTER_NODE(DestroyEntityNode, "Entity", GraphType::Scripting);
	GK_REGISTER_NODE(GetChildCountNode, "Entity", GraphType::Scripting);
	GK_REGISTER_NODE(SelfNode, "Entity", GraphType::Scripting);

	GK_REGISTER_NODE(StartNode, "Base", GraphType::Scripting);
	GK_REGISTER_NODE(UpdateNode, "Base", GraphType::Scripting);

	GK_REGISTER_NODE(OnCollisionEnterNode, "Physics", GraphType::Scripting);
	GK_REGISTER_NODE(OnCollisionExitNode, "Physics", GraphType::Scripting);
	GK_REGISTER_NODE(OnTriggerEnterNode, "Physics", GraphType::Scripting);
	GK_REGISTER_NODE(OnTriggerExitNode, "Physics", GraphType::Scripting);


	GK_REGISTER_NODE(StartTimerNode, "Time", GraphType::Scripting);
	GK_REGISTER_NODE(StopTimerNode, "Time", GraphType::Scripting);

	GK_REGISTER_NODE(ChangeLightColorNode, "Components", GraphType::Scripting);
	GK_REGISTER_NODE(LightNode, "Components", GraphType::Scripting);
	GK_REGISTER_NODE(PlayAudioNode, "Audio", GraphType::Scripting);

	GK_REGISTER_NODE(OutputPoseNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(CrossfadeNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(AdditiveNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(SequencePlayerNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(StateMachineNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(TransitionOutputNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(BlendSpaceNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(LayeredBlendPerBoneNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(RotateBoneNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(AnimationSwitchNode, "Animation", GraphType::Animation);
	GK_REGISTER_NODE(GetAnimationDurationNode, "Animation", GraphType::Animation);

	GK_REGISTER_NODE(MaterialOutputNode, "Material", GraphType::Material);
	GK_REGISTER_NODE(TextureSampleNode, "Material", GraphType::Material);

	GK_REGISTER_PARAMETER_TYPE(float, Float, [](std::string name, std::any& value) { UI::Property(name, std::any_cast<float&>(value)); });
	GK_REGISTER_PARAMETER_TYPE(int32_t, Int, [](std::string name, std::any& value) { UI::Property(name, std::any_cast<int32_t&>(value)); });
	GK_REGISTER_PARAMETER_TYPE(bool, Boolean, [](std::string name, std::any& value) { UI::Property(name, std::any_cast<bool&>(value)); });
	GK_REGISTER_PARAMETER_TYPE(gem::vec3, Vector3, [](std::string name, std::any& value) { UI::Property(name, std::any_cast<gem::vec3&>(value)); });
	GK_REGISTER_PARAMETER_TYPE(std::string, String, [](std::string name, std::any& value) { UI::Property(name, std::any_cast<std::string&>(value)); });
}
