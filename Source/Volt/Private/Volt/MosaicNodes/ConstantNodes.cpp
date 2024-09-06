#include "vtpch.h"
#include "Volt/MosaicNodes/ConstantNodes.h"

#include <Mosaic/NodeRegistry.h>

namespace Volt::MosaicNodes
{
	REGISTER_NODE_TEMPLATE(ConstantFloat, (ConstantNode<float, 0.f, Mosaic::ValueBaseType::Float, 1, "{5AAE4158-7282-43F9-9D6A-2259024E17B3}"_guid>));
	REGISTER_NODE_TEMPLATE(ConstantFloat2, (ConstantNode<glm::vec2, 0.f, Mosaic::ValueBaseType::Float, 2, "{3E369B4E-3945-4760-B81B-3A99F9AC3872}"_guid>));
	REGISTER_NODE_TEMPLATE(ConstantFloat3, (ConstantNode<glm::vec3, 0.f, Mosaic::ValueBaseType::Float, 3, "{8B7AEC16-DF23-40F8-AF64-67AF694EDDAC}"_guid>));
	REGISTER_NODE_TEMPLATE(ConstantFloat4, (ConstantNode<glm::vec4, 1.f, Mosaic::ValueBaseType::Float, 4, "{D79FF174-FF12-4070-8AEB-DA3B2F2F8AF4}"_guid>));

	REGISTER_NODE_TEMPLATE(ConstantInt, (ConstantNode<int32_t, 0, Mosaic::ValueBaseType::Int, 1, "{25A28EE3-73D1-4C60-81AD-31B32DEB5952}"_guid>));
	REGISTER_NODE_TEMPLATE(ConstantInt2, (ConstantNode<glm::ivec2, 0, Mosaic::ValueBaseType::Int, 2, "{8ABCB1A5-9BA2-4377-8E3C-66824D93D9B6}"_guid>));
	REGISTER_NODE_TEMPLATE(ConstantInt3, (ConstantNode<glm::ivec3, 0, Mosaic::ValueBaseType::Int, 3, "{FBB241A6-8C6D-4CEB-8F4E-63A14B765FBB}"_guid>));
	REGISTER_NODE_TEMPLATE(ConstantInt4, (ConstantNode<glm::ivec4, 0, Mosaic::ValueBaseType::Int, 4, "{271B6C2F-492D-4502-A71C-399835782446}"_guid>));

	REGISTER_NODE_TEMPLATE(Color3, (ColorNode<glm::vec3, 1.f, 3, "{A52F186F-07FD-4C77-80A2-436A509451FC}"_guid>));
	REGISTER_NODE_TEMPLATE(Color4, (ColorNode<glm::vec4, 1.f, 4, "{C032E7D5-D545-4DEF-8EC4-6A3980BC41B2}"_guid>));
}
