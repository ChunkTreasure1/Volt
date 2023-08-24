#pragma once

#include <unordered_map>

namespace Volt
{
	namespace RHI
	{
		class SamplerState;
	}

	enum class SamplerType
	{
		Linear,
		LinearClamp,
		LinearPoint,
		LinearPointClamp,

		Point,
		PointClamp,
		PointLinear,
		PointLinearClamp,

		Aniso2,
		Aniso4,
		Aniso8,
		Aniso16,

		Shadow
	};

	class SamplerLibrary
	{
	public:
		void Add(SamplerType samplerType);
		Ref<RHI::SamplerState> Get(SamplerType samplerType);

	private:
		std::unordered_map<SamplerType, Ref<RHI::SamplerState>> m_samplerStates;
	};
}
