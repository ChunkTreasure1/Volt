#include "vtpch.h"
#include "PhysXDebugger.h"

#include "PhysXInternal.h"

#include "Volt/Core/Application.h"
#include "Volt/Log/Log.h"

namespace Volt
{
	void PhysXDebugger::Initialize()
	{
		if (!Application::Get().IsRuntime())
		{
			myDebuggerData = CreateScope<PhysXData>();
			myDebuggerData->debugger = PxCreatePvd(PhysXInternal::GetFoundation());
			VT_CORE_ASSERT(myDebuggerData->debugger, "PxCreatePvd failed!");
		}
	}

	void PhysXDebugger::Shutdown()
	{
		if (!Application::Get().IsRuntime())
		{
			myDebuggerData->debugger->release();
			myDebuggerData = nullptr;
		}
	}

	void PhysXDebugger::StartDebugging(const std::filesystem::path& path, bool networkDebug)
	{
		if (!Application::Get().IsRuntime())
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
	}

	bool PhysXDebugger::IsDebugging()
	{
		if (!Application::Get().IsRuntime())
		{
			return myDebuggerData->debugger->isConnected();
		}

		return false;
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
		if (!Application::Get().IsRuntime())
		{
			return myDebuggerData->debugger;
		}
		return nullptr;
	}
}
