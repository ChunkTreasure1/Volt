#include "vtpch.h"
#include "PhysXDebugger.h"

#include "PhysXInternal.h"

#include "Volt/Log/Log.h"

namespace Volt
{
#ifndef VT_DIST

	void PhysXDebugger::Initialize()
	{
		myDebuggerData = CreateScope<PhysXData>();
		myDebuggerData->debugger = PxCreatePvd(PhysXInternal::GetFoundation());
		VT_CORE_ASSERT(myDebuggerData->debugger, "PxCreatePvd failed!");
	}

	void PhysXDebugger::Shutdown()
	{
		myDebuggerData->debugger->release();
		myDebuggerData = nullptr;
	}

	void PhysXDebugger::StartDebugging(const std::filesystem::path& path, bool networkDebug)
	{
		StopDebugging();

		if (!networkDebug)
		{
			myDebuggerData->transport = physx::PxDefaultPvdFileTransportCreate((path.string() + ".pxd2").c_str());
			myDebuggerData->debugger->connect(*myDebuggerData->transport, physx::PxPvdInstrumentationFlag::eALL);
		}
		else
		{
			myDebuggerData->transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
			myDebuggerData->debugger->connect(*myDebuggerData->transport, physx::PxPvdInstrumentationFlag::eALL);
		}
	}

	bool PhysXDebugger::IsDebugging()
	{
		return myDebuggerData->debugger->isConnected();
	}

	void PhysXDebugger::StopDebugging()
	{
		if (!IsDebugging())
		{
			return;
		}

		myDebuggerData->debugger->disconnect();
		myDebuggerData->transport->release();
	}

	physx::PxPvd* PhysXDebugger::GetDebugger()
	{
		return myDebuggerData->debugger;
	}

#else
	void PhysXDebugger::Initialize() {}
	void PhysXDebugger::Shutdown() {}
	void PhysXDebugger::StartDebugging(const std::filesystem::path&, bool) {}
	bool PhysXDebugger::IsDebugging() { return false; }
	void PhysXDebugger::StopDebugging() {}
	physx::PxPvd* PhysXDebugger::GetDebugger() { return nullptr; }
#endif
}