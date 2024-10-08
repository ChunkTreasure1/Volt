#pragma once

#include <EntitySystem/ComponentRegistry.h>

#include <cstdint>

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

	enum class CollisionDetectionType : uint32_t
	{
		Discrete = 0,
		Continuous,
		ContinuousSpeculative
	};

	static void ReflectType(TypeDesc<CollisionDetectionType>& reflect)
	{
		reflect.SetGUID("{E3CCC646-C301-492D-AD4B-A983B7B2BE72}"_guid);
		reflect.SetLabel("Collision Detection");
		reflect.SetDefaultValue(CollisionDetectionType::Discrete);
		reflect.AddConstant(CollisionDetectionType::Discrete, "discrete", "Discrete");
		reflect.AddConstant(CollisionDetectionType::Continuous, "continuous", "Continuous");
		reflect.AddConstant(CollisionDetectionType::ContinuousSpeculative, "continuousSpeculative", "Continuous Speculative");
	}

	REGISTER_ENUM(CollisionDetectionType);

	enum class BodyType : uint32_t
	{
		Static = 0,
		Dynamic
	};

	static void ReflectType(TypeDesc<BodyType>& reflect)
	{
		reflect.SetGUID("{98C6CC44-5B1A-4E20-BFA1-9DEBD31BE418}"_guid);
		reflect.SetLabel("Body Type");
		reflect.SetDefaultValue(BodyType::Static);
		reflect.AddConstant(BodyType::Static, "static", "Static");
		reflect.AddConstant(BodyType::Dynamic, "dynamic", "Dynamic");
	}

	REGISTER_ENUM(BodyType);

	enum class ClimbingMode : uint32_t
	{
		Normal = 0,
		Constrained
	};

	static void ReflectType(TypeDesc<ClimbingMode>& reflect)
	{
		reflect.SetGUID("{0871B88E-30A5-4082-B057-D088D4B1DD23}"_guid);
		reflect.SetLabel("Climbing Mode");
		reflect.SetDefaultValue(ClimbingMode::Normal);
		reflect.AddConstant(ClimbingMode::Normal, "normal", "Normal");
		reflect.AddConstant(ClimbingMode::Constrained, "constrained", "Constrained");
	}

	REGISTER_ENUM(ClimbingMode);

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
