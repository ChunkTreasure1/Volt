#include "gkpch.h"
#include "MathNodes.h"

namespace GraphKey
{
	void SlerpNode::Slerp()
	{
		const glm::vec3 a = GetInput<glm::vec3>(0);
		const glm::vec3 b = GetInput<glm::vec3>(1);
		const float t = GetInput<float>(2);

		const auto slerpedQuat = glm::slerp(glm::quat{ glm::radians(a) }, glm::quat{ glm::radians(b) }, t);
		SetOutputData(0, glm::degrees(glm::eulerAngles(slerpedQuat)));
	}

	void LerpNode::Lerp()
	{
		const glm::vec3 a = GetInput<glm::vec3>(0);
		const glm::vec3 b = GetInput<glm::vec3>(1);
		const float t = GetInput<float>(2);

		const auto result = glm::mix(a, b, t);
		SetOutputData(0, result);
	}
}

