#include "vtpch.h"
#include "PhysXInternal.h"

#include "Volt/Log/Log.h"

#include "Volt/Physics/PhysXDebugger.h"
#include "Volt/Physics/CookingFactory.h"

namespace Volt
{
	void PhysXInternal::Initialize()
	{
		VT_CORE_ASSERT(!myPhysXData, "PhysX should only be initiaize once!");

		myPhysXData = CreateScope<PhysXData>();
		myPhysXData->physxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, myPhysXData->allocator, myPhysXData->errorCallback);
		VT_CORE_ASSERT(myPhysXData->physxFoundation, "PxCreateFoundation failed!");

		physx::PxTolerancesScale scale = {};
		scale.length = 100;
		scale.speed = 1000;

		PhysXDebugger::Initialize();

#ifdef VT_DEBUG
		constexpr bool trackMemoryAllocations = true;
#else
		constexpr bool trackMemoryAllocations = false;
#endif

		myPhysXData->physxSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *myPhysXData->physxFoundation, scale, trackMemoryAllocations, PhysXDebugger::GetDebugger());
		VT_CORE_ASSERT(myPhysXData->physxSDK, "PxCreatePhysics failed!");

		bool extensionsLoaded = PxInitExtensions(*myPhysXData->physxSDK, PhysXDebugger::GetDebugger());
		extensionsLoaded;
		VT_CORE_ASSERT(extensionsLoaded, "Failed to initialize PhysX extensions");

		myPhysXData->physxCPUDispatcher = physx::PxDefaultCpuDispatcherCreate(4);

#ifdef VT_DEBUG
		PxSetAssertHandler(myPhysXData->assertHandler);
#endif
		// Initialize cooking
		CookingFactory::Initialize();
	}

	void PhysXInternal::Shutdown()
	{
		CookingFactory::Shutdown();

		myPhysXData->physxCPUDispatcher->release();
		myPhysXData->physxCPUDispatcher = nullptr;

		PxCloseExtensions();

		PhysXDebugger::StopDebugging();

		myPhysXData->physxSDK->release();
		myPhysXData->physxSDK = nullptr;

		PhysXDebugger::Shutdown();

		myPhysXData = nullptr;
	}

	physx::PxFoundation& PhysXInternal::GetFoundation()
	{
		return *myPhysXData->physxFoundation;
	}

	physx::PxPhysics& PhysXInternal::GetPhysXSDK()
	{
		return *myPhysXData->physxSDK;
	}

	physx::PxCpuDispatcher* PhysXInternal::GetCPUDispatcher()
	{
		return myPhysXData->physxCPUDispatcher;
	}

	physx::PxDefaultAllocator& PhysXInternal::GetAllocator()
	{
		return myPhysXData->allocator;
	}

	physx::PxFilterFlags PhysXInternal::FilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void*, physx::PxU32)
	{
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return physx::PxFilterFlag::eDEFAULT;
		}

		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		if (filterData0.word2 == (uint32_t)CollisionDetectionType::Continuous || filterData1.word2 == (uint32_t)CollisionDetectionType::Continuous)
		{
			pairFlags |= physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;
			pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;
		}

		if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
			return physx::PxFilterFlag::eDEFAULT;
		}

		return physx::PxFilterFlag::eSUPPRESS;
	}

	physx::PxBroadPhaseType::Enum PhysXInternal::VoltToPhysXBroadphase(BroadphaseType type)
	{
		switch (type)
		{
			case BroadphaseType::SweepAndPrune: return physx::PxBroadPhaseType::eSAP;
			case BroadphaseType::MultiBoxPrune: return physx::PxBroadPhaseType::eMBP;
			case BroadphaseType::AutomaticBoxPrune: return physx::PxBroadPhaseType::eABP;
			default:
				break;
		}

		return physx::PxBroadPhaseType::eABP;
	}

	physx::PxFrictionType::Enum PhysXInternal::VoltToPhysXFrictionType(FrictionType type)
	{
		switch (type)
		{
			case FrictionType::Patch: return physx::PxFrictionType::ePATCH;
			case FrictionType::OneDirectional: return physx::PxFrictionType::eONE_DIRECTIONAL;
			case FrictionType::TwoDirectional: return physx::PxFrictionType::eTWO_DIRECTIONAL;
			default:
				break;
		}

		return physx::PxFrictionType::ePATCH;
	}

	void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		const char* errorMessage = nullptr;

		switch (code)
		{
			case physx::PxErrorCode::eNO_ERROR:				errorMessage = "No Error"; break;
			case physx::PxErrorCode::eDEBUG_INFO:			errorMessage = "Info"; break;
			case physx::PxErrorCode::eDEBUG_WARNING:		errorMessage = "Warning"; break;
			case physx::PxErrorCode::eINVALID_PARAMETER:	errorMessage = "Invalid Parameter"; break;
			case physx::PxErrorCode::eINVALID_OPERATION:	errorMessage = "Invalid Operation"; break;
			case physx::PxErrorCode::eOUT_OF_MEMORY:		errorMessage = "Out Of Memory"; break;
			case physx::PxErrorCode::eINTERNAL_ERROR:		errorMessage = "Internal Error"; break;
			case physx::PxErrorCode::eABORT:				errorMessage = "Abort"; break;
			case physx::PxErrorCode::ePERF_WARNING:			errorMessage = "Performance Warning"; break;
			case physx::PxErrorCode::eMASK_ALL:				errorMessage = "Unknown Error"; break;
		}

		switch (code)
		{
			case physx::PxErrorCode::eNO_ERROR:
			case physx::PxErrorCode::eDEBUG_INFO:
				VT_CORE_INFO("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
				break;
			case physx::PxErrorCode::eDEBUG_WARNING:
			case physx::PxErrorCode::ePERF_WARNING:
				VT_CORE_INFO("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
				break;
			case physx::PxErrorCode::eINVALID_PARAMETER:
			case physx::PxErrorCode::eINVALID_OPERATION:
			case physx::PxErrorCode::eOUT_OF_MEMORY:
			case physx::PxErrorCode::eINTERNAL_ERROR:
				VT_CORE_ERROR("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
				break;
			case physx::PxErrorCode::eABORT:
			case physx::PxErrorCode::eMASK_ALL:
				VT_CORE_ERROR("[PhysX]: {0}: {1} at {2} ({3})", errorMessage, message, file, line);
				VT_ASSERT(false, "");
				break;
		}
	}

	void PhysicsAssertHandler::operator()(const char* exp, const char* file, int line, bool&)
	{
		VT_CORE_ERROR("[PhysX Error]: {0}:{1} - {2}", file, line, exp);
	}
}
