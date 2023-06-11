#include "gkpch.h"
#include "GraphKey/Registry.h"

#include "GraphKey/Nodes/MathNodes.h"
#include "GraphKey/Nodes/LogicNodes.h"

namespace GraphKey
{
	GK_REGISTER_NODE(BranchNode, "Logic", GraphType::All);
	GK_REGISTER_NODE(DoOnceNode, "Logic", GraphType::All);
	GK_REGISTER_NODE(FlipFlopNode, "Logic", GraphType::All);
	GK_REGISTER_NODE(NotNode, "Logic", GraphType::All);
	GK_REGISTER_NODE(AndNode, "Logic", GraphType::All);
	GK_REGISTER_NODE(OrNode, "Logic", GraphType::All);

	GK_REGISTER_NODE(ForRangedNode, "Logic", GraphType::Scripting);

	GK_REGISTER_NODE(SlerpNode, "Math", GraphType::All);
	GK_REGISTER_NODE(LerpNode, "Math", GraphType::All);

	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeFloat, "Math", GraphType::All, (DivisionNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeVector2, "Math", GraphType::All, (DivisionNode<glm::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeVector3, "Math", GraphType::All, (DivisionNode<glm::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeVector4, "Math", GraphType::All, (DivisionNode<glm::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(DivisionNodeInt, "Math", GraphType::All, (DivisionNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(MakeVector2Node, "Math", GraphType::All, (MakeVectorNode<glm::vec2, 2>));
	GK_REGISTER_NODE_SPECIALIZED(MakeVector3Node, "Math", GraphType::All, (MakeVectorNode<glm::vec3, 3>));
	GK_REGISTER_NODE_SPECIALIZED(MakeVector4Node, "Math", GraphType::All, (MakeVectorNode<glm::vec4, 4>));

	GK_REGISTER_NODE_SPECIALIZED(BreakVector2Node, "Math", GraphType::All, (BreakVectorNode<glm::vec2, 2>));
	GK_REGISTER_NODE_SPECIALIZED(BreakVector3Node, "Math", GraphType::All, (BreakVectorNode<glm::vec3, 3>));
	GK_REGISTER_NODE_SPECIALIZED(BreakVector4Node, "Math", GraphType::All, (BreakVectorNode<glm::vec4, 4>));

	GK_REGISTER_NODE_SPECIALIZED(NormalizeVector2Node, "Math", GraphType::All, (NormalizeVectorNode<glm::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(NormalizeVector3Node, "Math", GraphType::All, (NormalizeVectorNode<glm::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(NormalizeVector4Node, "Math", GraphType::All, (NormalizeVectorNode<glm::vec4>));

	GK_REGISTER_NODE_SPECIALIZED(Vector2LengthNode, "Math", GraphType::All, (LengthVectorNode<glm::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(Vector3LengthNode, "Math", GraphType::All, (LengthVectorNode<glm::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(Vector4LengthNode, "Math", GraphType::All, (LengthVectorNode<glm::vec4>));

	GK_REGISTER_NODE_SPECIALIZED(EqualNodeFloat, "Logic", GraphType::All, (EqualNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(EqualNodeVector2, "Logic", GraphType::All, (EqualNode<glm::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(EqualNodeVector3, "Logic", GraphType::All, (EqualNode<glm::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(EqualNodeVector4, "Logic", GraphType::All, (EqualNode<glm::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(EqualNodeInt, "Logic", GraphType::All, (EqualNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(AddNodeFloat, "Math", GraphType::All, (AddNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(AddNodeVector2, "Math", GraphType::All, (AddNode<glm::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(AddNodeVector3, "Math", GraphType::All, (AddNode<glm::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(AddNodeVector4, "Math", GraphType::All, (AddNode<glm::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(AddNodeInt, "Math", GraphType::All, (AddNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeFloat, "Math", GraphType::All, (SubtractNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeVector2, "Math", GraphType::All, (SubtractNode<glm::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeVector3, "Math", GraphType::All, (SubtractNode<glm::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeVector4, "Math", GraphType::All, (SubtractNode<glm::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(SubtractNodeInt, "Math", GraphType::All, (SubtractNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeFloat, "Math", GraphType::All, (MultiplyNode<float>));
	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeVector2, "Math", GraphType::All, (MultiplyNode<glm::vec2>));
	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeVector3, "Math", GraphType::All, (MultiplyNode<glm::vec3>));
	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeVector4, "Math", GraphType::All, (MultiplyNode<glm::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(MultiplyNodeInt, "Math", GraphType::All, (MultiplyNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(LessOrEqualNodeFloat, "Logic", GraphType::All, (LessOrEqualNode<float>));
	//GK_REGISTER_NODE_SPECIALIZED(LessOrEqualNodeVector2, "Logic", GraphType::All, (LessOrEqualNode<glm::vec2>));
	//GK_REGISTER_NODE_SPECIALIZED(LessOrEqualNodeVector3, "Logic", GraphType::All, (LessOrEqualNode<glm::vec3>));
	//GK_REGISTER_NODE_SPECIALIZED(LessOrEqualNodeVector4, "Logic", GraphType::All, (LessOrEqualNode<glm::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(LessOrEqualNodeInt, "Logic", GraphType::All, (LessOrEqualNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(GreaterOrEqualNodeFloat, "Logic", GraphType::All, (GreaterOrEqualNode<float>));
	//GK_REGISTER_NODE_SPECIALIZED(GreaterOrEqualNodeVector2, "Logic", GraphType::All, (GreaterOrEqualNode<glm::vec2>));
	//GK_REGISTER_NODE_SPECIALIZED(GreaterOrEqualNodeVector3, "Logic", GraphType::All, (GreaterOrEqualNode<glm::vec3>));
	//GK_REGISTER_NODE_SPECIALIZED(GreaterOrEqualNodeVector4, "Logic", GraphType::All, (GreaterOrEqualNode<glm::vec4>));
	GK_REGISTER_NODE_SPECIALIZED(GreaterOrEqualNodeInt, "Logic", GraphType::All, (GreaterOrEqualNode<int32_t>));

	GK_REGISTER_NODE_SPECIALIZED(BoolToFloat, "Conversions", GraphType::All, (ConvertFromToNode<bool, float>));
}
