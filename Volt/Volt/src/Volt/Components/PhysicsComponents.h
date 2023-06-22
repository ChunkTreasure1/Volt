#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Physics/PhysicsEnums.h"

#include <Wire/Serialization.h>
#include <glm/glm.hpp>

namespace Volt
{
	SERIALIZE_COMPONENT((struct CharacterControllerComponent
	{
		PROPERTY(Name = Climbing Mode, SpecialType = Enum) ClimbingMode climbingMode = ClimbingMode::Normal;

		PROPERTY(Name = Slope Limit) float slopeLimit = 20.f;
		PROPERTY(Name = Invisible Wall Height) float invisibleWallHeight = 200.f;
		PROPERTY(Name = Max Jump Height) float maxJumpHeight = 100.f;
		PROPERTY(Name = Contact Offset) float contactOffset = 1.f;
		PROPERTY(Name = Step Offset) float stepOffset = 10.f;
		PROPERTY(Name = Density) float density = 1.f;
		PROPERTY(Name = Layer) uint32_t layer = 0;
		PROPERTY(Name = Has Gravity) bool hasGravity = true;

		inline CharacterControllerComponent(ClimbingMode aClimbingMode = ClimbingMode::Normal, float aSlopeLimit = 20.f, float aInvisibleWallHeight = 200.f, float aMaxJumpHeight = 100.f,
			float aContactOffset = 1.f, float aStepOffset = 10.f, float aDensity = 1.f, uint32_t aLayer = 0, bool aHasGravity = true)
			: climbingMode(aClimbingMode), slopeLimit(aSlopeLimit), invisibleWallHeight(aInvisibleWallHeight), maxJumpHeight(aMaxJumpHeight), contactOffset(aContactOffset),
			stepOffset(aStepOffset), density(aDensity), layer(aLayer), hasGravity(aHasGravity)
		{ }

		CREATE_COMPONENT_GUID("{DC5C002A-B72E-42A0-83FC-FFBE1FB2DEF2}"_guid);
	}), CharacterControllerComponent);

	struct LastRigidbodyData
	{
		BodyType bodyType = BodyType::Static;
		uint32_t layerId = 0;
		float mass = 1.f;
		float linearDrag = 0.01f;
		float angularDrag = 0.05f;
		bool disableGravity = false;
		bool isKinematic = false;

		CollisionDetectionType collisionType = CollisionDetectionType::Discrete;
		uint32_t lockFlags = 0;
	};

	SERIALIZE_COMPONENT((struct RigidbodyComponent
	{
		PROPERTY(Name = Type, SpecialType = Enum) BodyType bodyType = BodyType::Static;
		PROPERTY(Name = Layer ID) uint32_t layerId = 0;
		PROPERTY(Name = Mass) float mass = 1.f;
		PROPERTY(Name = Linear Drag) float linearDrag = 0.01f;
		PROPERTY(Name = Lock Flags) uint32_t lockFlags = 0; // #TODO: add enum support
		PROPERTY(Name = Angular Drag) float angularDrag = 0.05f;
		PROPERTY(Name = Collision Type, SpecialType = Enum) CollisionDetectionType collisionType = CollisionDetectionType::Discrete;

		PROPERTY(Name = Disable Graivity) bool disableGravity = false;
		PROPERTY(Name = Is Kinematic) bool isKinematic = false;

		inline RigidbodyComponent(BodyType aBodyType = BodyType::Static, uint32_t aLayerId = 0, float aMass = 1.f, float aLinearDrag = 0.01f, uint32_t aLockFlags = 0,
			float aAngularDrag = 0.05f, bool aDisableGravity = false, bool aIsKinematic = false, CollisionDetectionType aCollisionType = CollisionDetectionType::Discrete)
			: bodyType(aBodyType), layerId(aLayerId), mass(aMass), linearDrag(aLinearDrag), lockFlags(aLockFlags), angularDrag(aAngularDrag), disableGravity(aDisableGravity),
			isKinematic(aIsKinematic), collisionType(aCollisionType)
		{
		}

		CREATE_COMPONENT_GUID("{460B7722-00C0-48BE-8B3E-B549BCC9269B}"_guid);
	}), RigidbodyComponent);

	struct LastBoxColliderData
	{
		glm::vec3 halfSize = { 50.f, 50.f, 50.f };
		glm::vec3 offset = { 0.f, 0.f, 0.f };
		bool isTrigger = false;
		AssetHandle material = Asset::Null();
	};

	SERIALIZE_COMPONENT((struct BoxColliderComponent
	{
		PROPERTY(Name = Half Size) glm::vec3 halfSize = { 50.f, 50.f, 50.f };
		PROPERTY(Name = Offset) glm::vec3 offset = { 0.f, 0.f, 0.f };
		PROPERTY(Name = Is Trigger) bool isTrigger = false;
		PROPERTY(Name = Physics Material) AssetHandle material = Asset::Null();

		bool added = false;

		inline BoxColliderComponent(const glm::vec3& aHalfSize = { 50.f, 50.f, 50.f }, const glm::vec3& aOffset = { 0.f }, bool aIsTrigger = false, AssetHandle aMaterial = Asset::Null())
			: halfSize(aHalfSize), offset(aOffset), isTrigger(aIsTrigger), material(aMaterial)
		{
		}

		CREATE_COMPONENT_GUID("{29707475-D536-4DA4-8D3A-A98948C89A5}"_guid);
	}), BoxColliderComponent);

	struct LastSphereColliderData
	{
		float radius = 50.f;
		glm::vec3 offset = { 0.f, 0.f, 0.f };
		bool isTrigger = false;
		AssetHandle material = Asset::Null();
	};

	SERIALIZE_COMPONENT((struct SphereColliderComponent
	{
		PROPERTY(Name = Radius) float radius = 50.f;
		PROPERTY(Name = Offset) glm::vec3 offset = { 0.f, 0.f, 0.f };
		PROPERTY(Name = Is Trigger) bool isTrigger = false;
		PROPERTY(Name = Physics Material) AssetHandle material = Asset::Null();

		bool added = false;

		inline SphereColliderComponent(float aRadius = 50.f, const glm::vec3& aOffset = { 0.f }, bool aIsTrigger = false, AssetHandle aMaterial = Asset::Null())
			: radius(aRadius), offset(aOffset), isTrigger(aIsTrigger), material(aMaterial)
		{
		}

		CREATE_COMPONENT_GUID("{90246BCE-FF83-41A2-A076-AB0A947C0D6A}"_guid);
	}), SphereColliderComponent);

	struct LastCapsuleColliderData
	{
		float radius = 50.f;
		float height = 50.f;
		glm::vec3 offset = { 0.f, 0.f, 0.f };
		bool isTrigger = false;
		AssetHandle material = Asset::Null();
	};

	SERIALIZE_COMPONENT((struct CapsuleColliderComponent
	{
		PROPERTY(Name = Radius) float radius = 50.f;
		PROPERTY(Name = Height) float height = 50.f;
		PROPERTY(Name = Offset) glm::vec3 offset = { 0.f, 0.f, 0.f };
		PROPERTY(Name = Is Trigger) bool isTrigger = false;
		PROPERTY(Name = Physics Material) AssetHandle material = Asset::Null();

		bool added = false;

		inline CapsuleColliderComponent(float aRadius = 50.f, float aHeight = 50.f, const glm::vec3& aOffset = { 0.f }, bool aIsTrigger = false, AssetHandle aMaterial = Asset::Null())
			: radius(aRadius), height(aHeight), offset(aOffset), isTrigger(aIsTrigger), material(aMaterial)
		{
		}

		CREATE_COMPONENT_GUID("{54A48952-7A77-492B-8A9C-2440D82EE5E2}"_guid);
	}), CapsuleColliderComponent);

	struct LastMeshColliderData
	{
		AssetHandle colliderMesh;
		bool isConvex = false;
		bool isTrigger = false;
		AssetHandle material = Asset::Null();
		int32_t subMeshIndex = -1;
	};

	SERIALIZE_COMPONENT((struct MeshColliderComponent
	{
		PROPERTY(Name = Collider Mesh, SpecialType = Asset, AssetType = Mesh) AssetHandle colliderMesh = Asset::Null();
		PROPERTY(Name = Physics Material) AssetHandle material = Asset::Null();
		PROPERTY(Name = Sub Mesh Index) int32_t subMeshIndex = -1;
		PROPERTY(Name = Is Convex) bool isConvex = true;
		PROPERTY(Name = Is Trigger) bool isTrigger = false;

		bool added = false;

		inline MeshColliderComponent(AssetHandle aColliderMesh = Asset::Null(), bool aIsConvex = false, bool aIsTrigger = false, AssetHandle aMaterial = Asset::Null(), int32_t aSubMeshIndex = -1)
			: colliderMesh(aColliderMesh), isConvex(aIsConvex), isTrigger(aIsTrigger), material(aMaterial), subMeshIndex(aSubMeshIndex)
		{
		}

		CREATE_COMPONENT_GUID("{E709C708-ED3C-4F68-BC1D-2FE32B897722}"_guid);
	}), MeshColliderComponent);
}
