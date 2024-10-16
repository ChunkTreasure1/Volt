#pragma once

#include "Volt/Physics/PhysicsEnums.h"
#include "Volt/Asset/AssetTypes.h"

#include <EntitySystem/Scripting/ECSAccessBuilder.h>

#include <AssetSystem/Asset.h>

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

		inline CharacterControllerComponent() = default;

		inline CharacterControllerComponent(ClimbingMode aClimbingMode, float aSlopeLimit, float aInvisibleWallHeight, float aMaxJumpHeight,
			float aContactOffset, float aStepOffset, float aDensity, uint32_t aLayer, bool aHasGravity)
			: climbingMode(aClimbingMode), slopeLimit(aSlopeLimit), invisibleWallHeight(aInvisibleWallHeight), maxJumpHeight(aMaxJumpHeight), contactOffset(aContactOffset),
			stepOffset(aStepOffset), density(aDensity), layer(aLayer), hasGravity(aHasGravity)
		{ 
			layer = aLayer;
		} 

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
			reflect.SetOnCreateCallback(&CharacterControllerComponent::OnCreate);
			reflect.SetOnDestroyCallback(&CharacterControllerComponent::OnDestroy);
			reflect.SetOnTransformChangedCallback(&CharacterControllerComponent::OnTransformChanged);
		}

		REGISTER_COMPONENT(CharacterControllerComponent);

	private:
		using PhysicsEntity = ECS::Access
			::Read<CharacterControllerComponent>
			::As<ECS::Type::Entity>;

		using PhysicsTransformEntity = ECS::Access
			::Read<TransformComponent>
			::With<CharacterControllerComponent>
			::As<ECS::Type::Entity>;

		static void OnCreate(PhysicsEntity entity);
		static void OnDestroy(PhysicsEntity entity);
		static void OnTransformChanged(PhysicsTransformEntity entity);
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
			reflect.AddMember(&BoxColliderComponent::material, "material", "Material", "", Asset::Null(), AssetTypes::PhysicsMaterial);
			reflect.SetOnCreateCallback(&BoxColliderComponent::OnCreate);
			reflect.SetOnDestroyCallback(&BoxColliderComponent::OnDestroy);
		}

		REGISTER_COMPONENT(BoxColliderComponent);

	private:
		using PhysicsEntity = ECS::Access
			::Write<BoxColliderComponent>
			::As<ECS::Type::Entity>;

		static void OnCreate(PhysicsEntity entity);
		static void OnDestroy(PhysicsEntity entity);
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
			reflect.AddMember(&SphereColliderComponent::material, "material", "Material", "", Asset::Null(), AssetTypes::PhysicsMaterial);
			reflect.SetOnCreateCallback(&SphereColliderComponent::OnCreate);
			reflect.SetOnDestroyCallback(&SphereColliderComponent::OnDestroy);
		}

		REGISTER_COMPONENT(SphereColliderComponent);
	
	private:
		using PhysicsEntity = ECS::Access
			::Write<SphereColliderComponent>
			::As<ECS::Type::Entity>;

		static void OnCreate(PhysicsEntity entity);
		static void OnDestroy(PhysicsEntity entity);
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
			reflect.AddMember(&CapsuleColliderComponent::material, "material", "Material", "", Asset::Null(), AssetTypes::PhysicsMaterial);
			reflect.SetOnCreateCallback(&CapsuleColliderComponent::OnCreate);
			reflect.SetOnDestroyCallback(&CapsuleColliderComponent::OnDestroy);
		}

		REGISTER_COMPONENT(CapsuleColliderComponent);

	private:
		using PhysicsEntity = ECS::Access
			::Write<CapsuleColliderComponent>
			::As<ECS::Type::Entity>;

		static void OnCreate(PhysicsEntity entity);
		static void OnDestroy(PhysicsEntity entity);
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
			: colliderMesh(aColliderMesh), material(aMaterial), subMeshIndex(aSubMeshIndex), isConvex(aIsConvex), isTrigger(aIsTrigger)
		{
		}

		static void ReflectType(TypeDesc<MeshColliderComponent>& reflect)
		{
			reflect.SetGUID("{E709C708-ED3C-4F68-BC1D-2FE32B897722}"_guid);
			reflect.SetLabel("Mesh Collider Component");
			reflect.AddMember(&MeshColliderComponent::colliderMesh, "colliderMesh", "Collider Mesh", "", Asset::Null(), AssetTypes::Mesh);
			reflect.AddMember(&MeshColliderComponent::material, "material", "Material", "", Asset::Null(), AssetTypes::PhysicsMaterial);
			reflect.AddMember(&MeshColliderComponent::subMeshIndex, "subMeshIndex", "subMeshIndex", "", -1);
			reflect.AddMember(&MeshColliderComponent::isConvex, "isConvex", "Is Convex", "", true);
			reflect.AddMember(&MeshColliderComponent::isTrigger, "isTrigger", "Is Trigger", "", false);
			reflect.SetOnCreateCallback(&MeshColliderComponent::OnCreate);
			reflect.SetOnDestroyCallback(&MeshColliderComponent::OnDestroy);
		}

		REGISTER_COMPONENT(MeshColliderComponent);

	private:
		using PhysicsEntity = ECS::Access
			::Write<MeshColliderComponent>
			::As<ECS::Type::Entity>;

		static void OnCreate(PhysicsEntity entity);
		static void OnDestroy(PhysicsEntity entity);
	};
}
