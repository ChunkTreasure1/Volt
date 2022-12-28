#include "gkpch.h"

#include "GraphKey/Registry.h"

#include "GraphKey/Nodes/BaseNodes.h"

#include "GraphKey/Nodes/InputNodes.h"
#include "GraphKey/Nodes/MathNodes.h"
#include "GraphKey/Nodes/PrintNodes.h"
#include "GraphKey/Nodes/UtilityNodes.h"

#include "GraphKey/Nodes/Entity/TransformNodes.h"
#include "GraphKey/Nodes/Entity/BaseEntityNodes.h"

namespace GraphKey
{
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeFloat, "Utility", (ToStringNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt, "Utility", (ToStringNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(AddNodeFloat, "Math", (AddNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(AddNodeVector2, "Math", (AddNode<gem::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(AddNodeVector3, "Math", (AddNode<gem::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(AddNodeVector4, "Math", (AddNode<gem::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(AddNodeInt, "Math", (AddNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeFloat, "Math", (SubtractNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeVector2, "Math", (SubtractNode<gem::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeVector3, "Math", (SubtractNode<gem::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeVector4, "Math", (SubtractNode<gem::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeInt, "Math", (SubtractNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeFloat, "Math", (MultiplyNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeVector2, "Math", (MultiplyNode<gem::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeVector3, "Math", (MultiplyNode<gem::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeVector4, "Math", (MultiplyNode<gem::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeInt, "Math", (MultiplyNode<int32_t>));
	
	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeFloat, "Math", (DivisionNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeVector2, "Math", (DivisionNode<gem::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeVector3, "Math", (DivisionNode<gem::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeVector4, "Math", (DivisionNode<gem::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeInt, "Math", (DivisionNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(ComposeVector2Node, "Math", (ComposeVectorNode<gem::vec2, 2>));
	GK_REGISTER_NODE_SPECIALIZED(ComposeVector3Node, "Math", (ComposeVectorNode<gem::vec3, 3>));
	GK_REGISTER_NODE_SPECIALIZED(ComposeVector4Node, "Math", (ComposeVectorNode<gem::vec4, 4>));

	GK_REGISTER_NODE_SPECIALIZED(DecomposeVector2Node, "Math", (DecomposeVectorNode<gem::vec2, 2>));
	GK_REGISTER_NODE_SPECIALIZED(DecomposeVector3Node, "Math", (DecomposeVectorNode<gem::vec3, 3>));
	GK_REGISTER_NODE_SPECIALIZED(DecomposeVector4Node, "Math", (DecomposeVectorNode<gem::vec4, 4>));

	GK_REGISTER_NODE(PrintNode, "Utility");

	GK_REGISTER_NODE(KeyPressedNode, "Input");
	GK_REGISTER_NODE(KeyReleasedNode, "Input");

	GK_REGISTER_NODE(SetEntityTransformNode, "Entity");
	GK_REGISTER_NODE(GetEntityTransformNode, "Entity");
	GK_REGISTER_NODE(CreateEntityNode, "Entity");

	GK_REGISTER_NODE(StartNode, "Base");
	GK_REGISTER_NODE(UpdateNode, "Base");
}