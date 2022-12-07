#pragma once

#include "Volt/Core/Base.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Volt
{
	class PhysXDebugger
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void StartDebugging(const std::filesystem::path& path, bool networkDebug = false);
		static bool IsDebugging();
		static void StopDebugging();

		static physx::PxPvd* GetDebugger();

	private:
		struct PhysXData
		{
			physx::PxPvd* debugger;
			physx::PxPvdTransport* transport;
		};
	
		inline static Scope<PhysXData> myDebuggerData;
	};
}