#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Physics/PhysicsEnums.h"

#include <glm/glm.hpp>

namespace Volt
{
	struct CharacterControllerComponent
	{
		ClimbingMode climbingMode = ClimbingMode::Normal;

		float slopeLimit = 20.f;
		float invisibleWallHeight = 200.f;
		float maxJumpHeight = 100.f;
		float contactOffset = 1.f;
		float stepOffset = 10.f;
		float density = 1.f;
		uint32_t layer = 0;
		bool hasGravity = true;

		inline CharacterControllerComponent(ClimbingMode aClimbingMode = ClimbingMode::Normal, float aSlopeLimit = 20.f, float aInvisibleWallHeight = 200.f, float aMaxJumpHeight = 100.f,
			float aContactOffset = 1.f, float aStepOffset = 10.f, float aDensity = 1.f, uint32_t aLayer = 0, bool aHasGravity = true)
			: climbingMode(aClimbingMode), slopeLimit(aSlopeLimit), invisibleWallHeight(aInvisibleWallHeight), maxJumpHeight(aMaxJumpHeight), contactOffset(aContactOffset),
			stepOffset(aStepOffset), density(aDensity), layer(aLayer), hasGravity(aHasGravity)
		{ }

		static void ReflectType(TypeDesc<CharacterControllerComponent>& reflect)
		{
			reflect.SetGUID("{DC5C002A-B72E-42A0-83FC-FFBE1FB2DEF2}"_guid);
			reflect.SetLabel("Character Controller Component");
			reflect.AddMember(&CharacterControllerComponent::climbingMode, "climbingMode", "Climbing Mode", "", ClimbingMode::Normal);
			reflect.AddMember(&CharacterControllerComponent::slopeLimit, "slopeLimit", "Slope Limit", "", 20.f);
			reflect.AddMember(&CharacterControllerComponent::invisibleWallHeight, "invisibleWallHeight", "Invisible Wall Height", "", 200.f);
			reflect.AddMember(&CharacterControllerComponent::maxJumpHeight, "maxJumpHeight", "Max Jump Height", "", 100.f);
			reflect.AddMember(&CharacterControllerComponent::contactOffset, "contactOffset", "Contact Offset", "", 1.f);
			reflect.AddMember(&CharacterControllerComponent::stepOffset, "stepOffset", "Step Offset", "", 10.f);
			reflect.AddMember(&CharacterControllerComponent::density, "density", "Density", "", 1.f);
			reflect.AddMember(&CharacterControllerComponent::layer, "layer", "Layer", "", 0);
			reflect.AddMember(&CharacterControllerComponent::hasGravity, "hasGravity", "Has Gravity", "", true);
		}

		REGISTER_COMPONENT(CharacterControllerComponent);
	};

	struct RigidbodyComponent
	{
		BodyType bodyType = BodyType::Static;
		uint32_t layerId = 0;
		float mass = 1.f;
		float linearDrag = 0.01f;
		uint32_t lockFlags = 0; // #TODO: add enum support
		float angularDrag = 0.05f;
		CollisionDetectionType collisionType = CollisionDetectionType::Discrete;

		bool disableGravity = false;
		bool isKinematic = false;

		inline RigidbodyComponent(BodyType aBodyType = BodyType::Static, uint32_t aLayerId = 0, float aMass = 1.f, float aLinearDrag = 0.01f, uint32_t aLockFlags = 0,
			float aAngularDrag = 0.05f, bool aDisableGravity = false, bool aIsKinematic = false, CollisionDetectionType aCollisionType = CollisionDetectionType::Discrete)
			: bodyType(aBodyType), layerId(aLayerId), mass(aMass), linearDrag(aLinearDrag), lockFlags(aLockFlags), angularDrag(aAngularDrag), disableGravity(aDisableGravity),
			isKinematic(aIsKinematic), collisionType(aCollisionType)
		{
		}

		static void ReflectType(TypeDesc<RigidbodyComponent>& reflect)
		{
			reflect.SetGUID("{460B7722-00C0-48BE-8B3E-B549BCC9269B}"_guid);
			reflect.SetLabel("Rigidbody Component");
			reflect.AddMember(&RigidbodyComponent::bodyType, "bodyType", "Body Type", "", BodyType::Static);
			reflect.AddMember(&RigidbodyComponent::layerId, "layerId", "Layer ID", "", 0);
			reflect.AddMember(&RigidbodyComponent::mass, "mass", "Mass", "", 1.f);
			reflect.AddMember(&RigidbodyComponent::linearDrag, "linearDrag", "Linear Drag", "", 0.01f);
			reflect.AddMember(&RigidbodyComponent::lockFlags, "lockFlags", "Lock Flags", "", 0);
			reflect.AddMember(&RigidbodyComponent::angularDrag, "angularDrag", "Angular Drag", "", 0.05f);
			reflect.AddMember(&RigidbodyComponent::collisionType, "collisionType", "Collision Type", "", CollisionDetectionType::Discrete);
			reflect.AddMember(&RigidbodyComponent::disableGravity, "disableGravity", "Disable Gravity", "", false);
			reflect.AddMember(&RigidbodyComponent::isKinematic, "isKinematic", "Is Kinematic", "", false);
		}

		REGISTER_COMPONENT(RigidbodyComponent);
	};

	struct BoxColliderComponent
	{
		glm::vec3 halfSize = { 50.f, 50.f, 50.f };
		glm::vec3 offset = { 0.f, 0.f, 0.f };
		bool isTrigger = false;
		AssetHandle material = Asset::Null();

		bool added = false;

		inline BoxColliderComponent(const glm::vec3& aHalfSize = { 50.f, 50.f, 50.f }, const glm::vec3& aOffset = { 0.f }, bool aIsTrigger = false, AssetHandle aMaterial = Asset::Null())
			: halfSize(aHalfSize), offset(aOffset), isTrigger(aIsTrigger), material(aMaterial)
		{
		}

		static void ReflectType(TypeDesc<BoxColliderComponent>& reflect)
		{
			reflect.SetGUID("{29707475-D536-4DA4-8D3A-A98948C89A5}"_guid);
			reflect.SetLabel("Box Collider Component");
			reflect.AddMember(&BoxColliderComponent::halfSize, "halfSize", "Half Size", "", glm::vec3{ 50.f });
			reflect.AddMember(&BoxColliderComponent::offset, "offset", "Offset", "", glm::vec3{ 0.f });
			reflect.AddMember(&BoxColliderComponent::isTrigger, "isTrigger", "Is Trigger", "", false);
			reflect.AddMember(&BoxColliderComponent::material, "material", "Material", "", Asset::Null(), AssetType::PhysicsMaterial);
		}

		REGISTER_COMPONENT(BoxColliderComponent);
	};

	struct SphereColliderComponent
	{
		float radius = 50.f;
		glm::vec3 offset = { 0.f, 0.f, 0.f };
		bool isTrigger = false;
		AssetHandle material = Asset::Null();

		bool added = false;

		inline SphereColliderComponent(float aRadius = 50.f, const glm::vec3& aOffset = { 0.f }, bool aIsTrigger = false, AssetHandle aMaterial = Asset::Null())
			: radius(aRadius), offset(aOffset), isTrigger(aIsTrigger), material(aMaterial)
		{
		}

		static void ReflectType(TypeDesc<SphereColliderComponent>& reflect)
		{
			reflect.SetGUID("{90246BCE-FF83-41A2-A076-AB0A947C0D6A}"_guid);
			reflect.SetLabel("Sphere Collider Component");
			reflect.AddMember(&SphereColliderComponent::radius, "radius", "Radius", "", 50.f);
			reflect.AddMember(&SphereColliderComponent::offset, "offset", "Offset", "", glm::vec3{ 0.f });
			reflect.AddMember(&SphereColliderComponent::isTrigger, "isTrigger", "Is Trigger", "", false);
			reflect.AddMember(&SphereColliderComponent::material, "material", "Material", "", Asset::Null(), AssetType::PhysicsMaterial);
		}

		REGISTER_COMPONENT(SphereColliderComponent);
	};

	struct CapsuleColliderComponent
	{
		float radius = 50.f;
		float height = 50.f;
		glm::vec3 offset = { 0.f, 0.f, 0.f };
		bool isTrigger = false;
		AssetHandle material = Asset::Null();

		bool added = false;

		inline CapsuleColliderComponent(float aRadius = 50.f, float aHeight = 50.f, const glm::vec3& aOffset = { 0.f }, bool aIsTrigger = false, AssetHandle aMaterial = Asset::Null())
			: radius(aRadius), height(aHeight), offset(aOffset), isTrigger(aIsTrigger), material(aMaterial)
		{
		}

		static void ReflectType(TypeDesc<CapsuleColliderComponent>& reflect)
		{
			reflect.SetGUID("{54A48952-7A77-492B-8A9C-2440D82EE5E2}"_guid);
			reflect.SetLabel("Capsule Collider Component");
			reflect.AddMember(&CapsuleColliderComponent::radius, "radius", "Radius", "", 50.f);
			reflect.AddMember(&CapsuleColliderComponent::height, "height", "Height", "", 50.f);
			reflect.AddMember(&CapsuleColliderComponent::offset, "offset", "Offset", "", glm::vec3{ 0.f });
			reflect.AddMember(&CapsuleColliderComponent::isTrigger, "isTrigger", "Is Trigger", "", false);
			reflect.AddMember(&CapsuleColliderComponent::material, "material", "Material", "", Asset::Null(), AssetType::PhysicsMaterial);
		}

		REGISTER_COMPONENT(CapsuleColliderComponent);
	};

	struct MeshColliderComponent
	{
		AssetHandle colliderMesh = Asset::Null();
		AssetHandle material = Asset::Null();
		int32_t subMeshIndex = -1;
		bool isConvex = true;
		bool isTrigger = false;

		bool added = false;

		inline MeshColliderComponent(AssetHandle aColliderMesh = Asset::Null(), bool aIsConvex = false, bool aIsTrigger = false, AssetHandle aMaterial = Asset::Null(), int32_t aSubMeshIndex = -1)
			: colliderMesh(aColliderMesh), isConvex(aIsConvex), isTrigger(aIsTrigger), material(aMaterial), subMeshIndex(aSubMeshIndex)
		{
		}

		static void ReflectType(TypeDesc<MeshColliderComponent>& reflect)
		{
			reflect.SetGUID("{E709C708-ED3C-4F68-BC1D-2FE32B897722}"_guid);
			reflect.SetLabel("Mesh Collider Component");
			reflect.AddMember(&MeshColliderComponent::colliderMesh, "colliderMesh", "Collider Mesh", "", Asset::Null(), AssetType::Mesh);
			reflect.AddMember(&MeshColliderComponent::material, "material", "Material", "", Asset::Null(), AssetType::PhysicsMaterial);
			reflect.AddMember(&MeshColliderComponent::subMeshIndex, "subMeshIndex", "subMeshIndex", "", -1);
			reflect.AddMember(&MeshColliderComponent::isConvex, "isConvex", "Is Convex", "", true);
			reflect.AddMember(&MeshColliderComponent::isTrigger, "isTrigger", "Is Trigger", "", false);
		}

		REGISTER_COMPONENT(MeshColliderComponent);
	};
}
