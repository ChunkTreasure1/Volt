#include "gkpch.h"

#include "GraphKey/Registry.h"

#include "GraphKey/Nodes/InputNodes.h"
#include "GraphKey/Nodes/MathNodes.h"
#include "GraphKey/Nodes/PrintNodes.h"
#include "GraphKey/Nodes/UtilityNodes.h"

namespace GraphKey
{
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeFloat, ToStringNode<float>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeDouble, ToStringNode<double>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt32, ToStringNode<int32_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeUInt32, ToStringNode<uint32_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt16, ToStringNode<int16_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeUInt16, ToStringNode<uint16_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeInt8, ToStringNode<int8_t>);
	GK_REGISTER_NODE_SPECIALIZED(ToStringNodeUInt8, ToStringNode<uint8_t>);

	GK_REGISTER_NODE(PrintNode);

	GK_REGISTER_NODE(AddNode);
	GK_REGISTER_NODE(SubtractNode);
	GK_REGISTER_NODE(MultiplyNode);
	GK_REGISTER_NODE(DivisionNode);

	GK_REGISTER_NODE(KeyPressedNode);
	GK_REGISTER_NODE(KeyReleasedNode);
}