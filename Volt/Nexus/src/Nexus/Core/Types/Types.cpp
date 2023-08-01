#include "nexuspch.h"
#include "Types.h"

#include <random>

namespace Nexus::TYPE
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());

	static std::uniform_int_distribution<TYPE::CLIENT_ID> s_UniformDistributionClientId;
	static std::uniform_int_distribution<TYPE::REP_ID> s_UniformDistributionRepId;
	static std::uniform_int_distribution<TYPE::RPC_ID> s_UniformDistributionRPCId;
	static std::uniform_int_distribution<TYPE::NOTIFY_ID> s_UniformDistributionNotifyId;
	static std::uniform_int_distribution<TYPE::NETSCENE_INSTANCE_ID> s_UniformDistributionSceneInstanceId;

	CLIENT_ID RandClientID()
	{
		return s_UniformDistributionClientId(s_Engine);
	}

	REP_ID RandRepID()
	{
		return s_UniformDistributionRepId(s_Engine);
	}

	RPC_ID RandRPCID()
	{
		return s_UniformDistributionRPCId(s_Engine);
	}

	NOTIFY_ID RandNotifyID()
	{
		return s_UniformDistributionNotifyId(s_Engine);
	}

	NETSCENE_INSTANCE_ID RandSceneInstanceID()
	{
		return s_UniformDistributionSceneInstanceId(s_Engine);
	}
}
