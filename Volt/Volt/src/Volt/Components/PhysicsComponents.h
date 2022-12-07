#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Physics/PhysicsEnums.h"

#include <Wire/Serialization.h>
#include <gem/gem.h>

namespace Volt
{
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
		PROPERTY(Name = Type, SpecialType = Enum) BodyType bodyType = BodyType::Static; // #TODO: add enum support
		PROPERTY(Name = Layer ID) uint32_t layerId = 0;
		PROPERTY(Name = Mass) float mass = 1.f;
		PROPERTY(Name = Linear Drag) float linearDrag = 0.01f;
		PROPERTY(Name = Lock Flags) uint32_t lockFlags = 0; // #TODO: add enum support
		PROPERTY(Name = Angular Drag) float angularDrag = 0.05f;
		PROPERTY(Name = Disable Graivity) bool disableGravity = false;
		PROPERTY(Name = Is Kinematic) bool isKinematic = false;

		PROPERTY(Name = Collision Type, SpecialType = Enum) CollisionDetectionType collisionType = CollisionDetectionType::Discrete; // #TODO: add enum support

		LastRigidbodyData lastRigidbodyData;

		void UpdateLast()
		{
			lastRigidbodyData.bodyType = (BodyType)bodyType;
			lastRigidbodyData.layerId = layerId;
			lastRigidbodyData.mass = mass;
			lastRigidbodyData.linearDrag = linearDrag;
			lastRigidbodyData.angularDrag = angularDrag;
			lastRigidbodyData.disableGravity = disableGravity;
			lastRigidbodyData.isKinematic = isKinematic;
			lastRigidbodyData.collisionType = (CollisionDetectionType)collisionType;
			lastRigidbodyData.lockFlags = lockFlags;
		}

		CREATE_COMPONENT_GUID("{460B7722-00C0-48BE-8B3E-B549BCC9269B}"_guid);
	}), RigidbodyComponent);

	struct LastBoxColliderData
	{
		gem::vec3 halfSize = { 50.f, 50.f, 50.f };
		gem::vec3 offset = { 0.f, 0.f, 0.f };
		bool isTrigger = false;
		AssetHandle material = Asset::Null();
	};

	SERIALIZE_COMPONENT((struct BoxColliderComponent
	{
		PROPERTY(Name = Half Size) gem::vec3 halfSize = { 50.f, 50.f, 50.f };
		PROPERTY(Name = Offset) gem::vec3 offset = { 0.f, 0.f, 0.f };
		PROPERTY(Name = Is Trigger) bool isTrigger = false;
		PROPERTY(Name = Physics Material) AssetHandle material = Asset::Null();

		LastBoxColliderData lastColliderData;
		bool added = false;

		void UpdateLast()
		{
			lastColliderData.halfSize = halfSize;
			lastColliderData.offset = offset;
			lastColliderData.isTrigger = isTrigger;
			lastColliderData.material = material;
		}

		CREATE_COMPONENT_GUID("{29707475-D536-4DA4-8D3A-A98948C89A5}"_guid);
	}), BoxColliderComponent);

	struct LastSphereColliderData
	{
		float radius = 50.f;
		gem::vec3 offset = { 0.f, 0.f, 0.f };
		bool isTrigger = false;
		AssetHandle material = Asset::Null();
	};

	SERIALIZE_COMPONENT((struct SphereColliderComponent
	{
		PROPERTY(Name = Radius) float radius = 50.f;
		PROPERTY(Name = Offset) gem::vec3 offset = { 0.f, 0.f, 0.f };
		PROPERTY(Name = Is Trigger) bool isTrigger = false;
		PROPERTY(Name = Physics Material) AssetHandle material = Asset::Null();

		LastSphereColliderData lastColliderData;
		bool added = false;

		void UpdateLast()
		{
			lastColliderData.radius = radius;
			lastColliderData.offset = offset;
			lastColliderData.isTrigger = isTrigger;
			lastColliderData.material = material;
		}

		CREATE_COMPONENT_GUID("{90246BCE-FF83-41A2-A076-AB0A947C0D6A}"_guid);
	}), SphereColliderComponent);

	struct LastCapsuleColliderData
	{
		float radius = 50.f;
		float height = 50.f;
		gem::vec3 offset = { 0.f, 0.f, 0.f };
		bool isTrigger = false;
		AssetHandle material = Asset::Null();
	};

	SERIALIZE_COMPONENT((struct CapsuleColliderComponent
	{
		PROPERTY(Name = Radius) float radius = 50.f;
		PROPERTY(Name = Height) float height = 50.f;
		PROPERTY(Name = Offset) gem::vec3 offset = { 0.f, 0.f, 0.f };
		PROPERTY(Name = Is Trigger) bool isTrigger = false;
		PROPERTY(Name = Physics Material) AssetHandle material = Asset::Null();

		LastCapsuleColliderData lastColliderData;
		bool added = false;

		void UpdateLast()
		{
			lastColliderData.radius = radius;
			lastColliderData.height = height;
			lastColliderData.offset = offset;
			lastColliderData.isTrigger = isTrigger;
			lastColliderData.material = material;
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
		PROPERTY(Name = Collider Mesh) AssetHandle colliderMesh;
		PROPERTY(Name = Is Convex) bool isConvex = false;
		PROPERTY(Name = Is Trigger) bool isTrigger = false;
		PROPERTY(Name = Physics Material) AssetHandle material = Asset::Null();
		PROPERTY(Name = Sub Mesh Index) int32_t subMeshIndex = -1;

		LastMeshColliderData lastColliderData;
		bool added = false;

		void UpdateLast()
		{
			lastColliderData.colliderMesh = colliderMesh;
			lastColliderData.isConvex = isConvex;
			lastColliderData.isTrigger = isTrigger;
			lastColliderData.material = material;
			lastColliderData.subMeshIndex = subMeshIndex;
		}

		CREATE_COMPONENT_GUID("{E709C708-ED3C-4F68-BC1D-2FE32B897722}"_guid);
	}), MeshColliderComponent);
}