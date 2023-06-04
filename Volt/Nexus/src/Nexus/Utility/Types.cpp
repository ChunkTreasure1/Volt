#include "nexuspch.h"
#include "Types.h"

#include <random>

namespace Nexus
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());

	static std::uniform_int_distribution<TYPE::CLIENT_ID> s_UniformDistributionClientId;
	static std::uniform_int_distribution<TYPE::REP_ID> s_UniformDistributionRepId;
	static std::uniform_int_distribution<TYPE::RPC_ID> s_UniformDistributionRPCId;

	TYPE::CLIENT_ID RandClientID()
	{
		return s_UniformDistributionClientId(s_Engine);
	}

	TYPE::REP_ID RandRepID()
	{
		return s_UniformDistributionRepId(s_Engine);
	}

	TYPE::REP_ID RandRPCID()
	{
		return s_UniformDistributionRPCId(s_Engine);
	}
}
