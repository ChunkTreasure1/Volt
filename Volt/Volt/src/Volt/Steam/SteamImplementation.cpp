#include "vtpch.h"
#include "SteamImplementation.h"

#include "Volt/Core/Profiling.h"

namespace Volt
{
	SteamImplementation::SteamImplementation()
	{
		if (SteamAPI_Init())
		{
			mySteamInitialized = true;
		}
	}

	SteamImplementation::~SteamImplementation()
	{
		SteamAPI_Shutdown();
	}

	std::string SteamImplementation::GetPersona() const
	{
		if (mySteamInitialized)
		{
			return SteamFriends()->GetPersonaName();
		}
		return "";
	}

	Scope<SteamImplementation> SteamImplementation::Create()
	{
		return CreateScope<SteamImplementation>();
	}
}
