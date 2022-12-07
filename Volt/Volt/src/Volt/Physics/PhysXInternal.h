#pragma once

#include "Volt/Core/Base.h"
#include "PhysicsSettings.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Volt
{
	class PhysicsErrorCallback : public physx::PxErrorCallback
	{
	public:
		void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
	};

	class PhysicsAssertHandler : public physx::PxAssertHandler
	{
		void operator()(const char* exp, const char* file, int line, bool& ignore) override;
	};

	class PhysXInternal
	{
	public:
		static void Initialize();
		static void Shutdown();
	
		static physx::PxFoundation& GetFoundation();
		static physx::PxPhysics& GetPhysXSDK();
		static physx::PxCpuDispatcher* GetCPUDispatcher();
		static physx::PxDefaultAllocator& GetAllocator();

		static physx::PxFilterFlags FilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
			physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

		static physx::PxBroadPhaseType::Enum VoltToPhysXBroadphase(BroadphaseType type);
		static physx::PxFrictionType::Enum VoltToPhysXFrictionType(FrictionType type);

	private:
		struct PhysXData
		{
			physx::PxFoundation* physxFoundation{};
			physx::PxDefaultCpuDispatcher* physxCPUDispatcher{};
			physx::PxPhysics* physxSDK{};

			physx::PxDefaultAllocator allocator{};
			PhysicsErrorCallback errorCallback;
			PhysicsAssertHandler assertHandler;
		};

		inline static Scope<PhysXData> myPhysXData;
	};
}