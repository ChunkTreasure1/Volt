#pragma once

#include "Volt/Core/Base.h"

#include <cstdint>
#include <Wire/Serialization.h>

namespace Volt
{
	enum class BroadphaseType : uint32_t
	{
		SweepAndPrune = 0,
		MultiBoxPrune,
		AutomaticBoxPrune
	};

	enum class FrictionType : uint32_t
	{
		Patch = 0,
		OneDirectional,
		TwoDirectional
	};

	enum class DebugType : uint32_t
	{
		DebugToFile = 0,
		LiveDebug
	};

	SERIALIZE_ENUM((enum class CollisionDetectionType : uint32_t
	{
		Discrete = 0,
		Continuous,
		ContinuousSpeculative
	}), CollisionDetectionType);

	SERIALIZE_ENUM((enum class BodyType : uint32_t
	{
		Static = 0,
		Dynamic
	}), BodyType);

	SERIALIZE_ENUM((enum class ClimbingMode : uint32_t
	{
		Normal = 0,
		Constrained

	}), ClimbingMode)

		enum class CookingResult
	{
		Success = 0,
		ZeroAreaTestFailed,
		PolygonLimitReached,
		LargeTriangle,
		Failure
	};

	enum class ForceMode : uint8_t
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum class ActorLockFlag : uint32_t
	{
		TranslationX = BIT(0), TranslationY = BIT(1), TranslationZ = BIT(2), Translation = TranslationX | TranslationY | TranslationZ,
		RotationX = BIT(3), RotationY = BIT(4), RotationZ = BIT(5), Rotation = RotationX | RotationY | RotationZ
	};

	enum class ColliderType : uint32_t
	{
		Box = 0,
		Sphere,
		Capsule,
		ConvexMesh,
		TriangleMesh
	};
}