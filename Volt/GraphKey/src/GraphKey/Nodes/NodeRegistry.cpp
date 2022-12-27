#include "gkpch.h"

#include "GraphKey/Registry.h"

#include "GraphKey/Nodes/BaseNodes.h"

#include "GraphKey/Nodes/InputNodes.h"
#include "GraphKey/Nodes/MathNodes.h"
#include "GraphKey/Nodes/PrintNodes.h"
#include "GraphKey/Nodes/UtilityNodes.h"

#include "GraphKey/Nodes/Entity/TransformNodes.h"

namespace GraphKey
{
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeFloat, "Utility", ToStringNode<float>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeDouble, "Utility", ToStringNode<double>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt32, "Utility", ToStringNode<int32_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeUInt32, "Utility", ToStringNode<uint32_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt16, "Utility", ToStringNode<int16_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeUInt16, "Utility", ToStringNode<uint16_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt8, "Utility", ToStringNode<int8_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeUInt8, "Utility", ToStringNode<uint8_t>);

	GK_REGISTER_NODE(PrintNode, "Utility");

	GK_REGISTER_NODE(AddNode, "Math");
	GK_REGISTER_NODE(SubtractNode, "Math");
	GK_REGISTER_NODE(MultiplyNode, "Math");
	GK_REGISTER_NODE(DivisionNode, "Math");

	GK_REGISTER_NODE(KeyPressedNode, "Input");
	GK_REGISTER_NODE(KeyReleasedNode, "Input");

	GK_REGISTER_NODE(SetEntityTransformNode, "Entity");
	GK_REGISTER_NODE(GetEntityTransformNode, "Entity");

	GK_REGISTER_NODE(StartNode, "Base");
	GK_REGISTER_NODE(UpdateNode, "Base");
}