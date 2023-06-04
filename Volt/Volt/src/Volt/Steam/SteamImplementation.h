#pragma once

#include "Volt/Core/Base.h"

#include <steam/steam_api.h>

#include <string>

namespace Volt
{
	class SteamImplementation
	{
	public:
		SteamImplementation();
		virtual ~SteamImplementation();

		std::string GetPersona() const;

		static Scope<SteamImplementation> Create();

	private:
		bool mySteamInitialized = false;
	};
}
