#include "gkpch.h"
#include "MathNodes.h"

namespace GraphKey
{
	void SlerpNode::Slerp()
	{
		const gem::vec3 a = GetInput<gem::vec3>(0);
		const gem::vec3 b = GetInput<gem::vec3>(1);
		const float t = GetInput<float>(2);

		const auto slerpedQuat = gem::slerp(gem::quat{ gem::radians(a) }, gem::quat{ gem::radians(b) }, t);
		SetOutputData(0, gem::degrees(gem::eulerAngles(slerpedQuat)));
	}

	void LerpNode::Lerp()
	{
		const gem::vec3 a = GetInput<gem::vec3>(0);
		const gem::vec3 b = GetInput<gem::vec3>(1);
		const float t = GetInput<float>(2);

		const auto result = gem::lerp(a, b, t);
		SetOutputData(0, result);
	}
}

