#include "vtpch.h"
#include "PhysicsShapes.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Core/Application.h"

#include "Volt/Scene/Entity.h"

#include "Volt/Components/CoreComponents.h"
#include "Volt/Components/PhysicsComponents.h"

#include "Volt/Physics/PhysicsMaterial.h"
#include "Volt/Physics/PhysXInternal.h"
#include "Volt/Physics/PhysXUtilities.h"
#include "Volt/Physics/PhysicsActor.h"

#include "Volt/Physics/MeshColliderCache.h"
#include "Volt/Physics/CookingFactory.h"

#include "Volt/Log/Log.h"

namespace Volt
{
	ColliderShape::ColliderShape(ColliderType type, Entity entity)
		: myColliderType(type), myEntity(entity)
	{
	}

	void ColliderShape::Release()
	{
		//myMaterial->release();
	}

	void ColliderShape::SetMaterial(Ref<PhysicsMaterial> material)
	{
		Ref<PhysicsMaterial> mat = material;
		if (!mat)
		{
			mat = CreateRef<PhysicsMaterial>(); //#TODO_Ivar: This will create issues later
			mat->staticFriction = 0.6f;
			mat->dynamicFriction = 0.6f;
			mat->bounciness = 0.3f;
		}

		if (myMaterial)
		{
			myMaterial->release();
		}

		myMaterial = PhysXInternal::GetPhysXSDK().createMaterial(mat->staticFriction, mat->dynamicFriction, mat->bounciness);
	}

	///// Box Collider /////
	BoxColliderShape::BoxColliderShape(BoxColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::Box, entity)
	{
		SetMaterial(nullptr); // #TODO: Implement actual materials

		Scene::TQS transform;

		if (entity.HasComponent<TransformComponent>())
		{
			transform = entity.GetScene()->GetWorldTQS(entity);
		}

		component.added = true;

		const glm::vec3 colliderSize = glm::abs(transform.scale * component.halfSize);
		physx::PxBoxGeometry geometry = physx::PxBoxGeometry(colliderSize.x, colliderSize.y, colliderSize.z);
		myShape = physx::PxRigidActorExt::createExclusiveShape(actor.GetActor(), geometry, *myMaterial);
		myShape->setSimulationFilterData(actor.GetFilterData());
		myShape->setQueryFilterData(actor.GetFilterData());
		myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !component.isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, !component.isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.isTrigger);
		myShape->setLocalPose(PhysXUtilities::ToPhysXTransform(component.offset * transform.scale, glm::vec3{ 0.f }));
		myShape->userData = this;
	}

	BoxColliderShape::~BoxColliderShape()
	{
	}

	void BoxColliderShape::SetHalfSize(const glm::vec3& halfSize)
	{
		Scene::TQS transform;

		if (myEntity.HasComponent<TransformComponent>())
		{
			transform = myEntity.GetScene()->GetWorldTQS(myEntity);
		}

		const glm::vec3 colliderSize = transform.scale * halfSize;
		physx::PxBoxGeometry geometry = physx::PxBoxGeometry(colliderSize.x, colliderSize.y, colliderSize.z);
		myShape->setGeometry(geometry);

		myEntity.GetComponent<BoxColliderComponent>().halfSize = halfSize;
	}

	void BoxColliderShape::SetTrigger(bool isTrigger)
	{
		myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, !isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

		myEntity.GetComponent<BoxColliderComponent>().isTrigger = isTrigger;
	}

	void BoxColliderShape::SetOffset(const glm::vec3& offset)
	{
		myShape->setLocalPose(PhysXUtilities::ToPhysXTransform(offset, glm::vec3{ 0.f }));
		myEntity.GetComponent<BoxColliderComponent>().offset = offset;
	}

	void BoxColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		myShape->setSimulationFilterData(filterData);
		myShape->setQueryFilterData(filterData);
	}

	void BoxColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		VT_CORE_ASSERT(actor, "Actor is null!");
		VT_CORE_ASSERT(myShape, "Shape is null!");
		actor->detachShape(*myShape);
	}

	///// Sphere Collider /////
	SphereColliderShape::SphereColliderShape(SphereColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::Sphere, entity)
	{
		component.added = true;

		SetMaterial(nullptr); // #TODO: Implement actual materials

		Scene::TQS transform;

		if (entity.HasComponent<TransformComponent>())
		{
			transform = entity.GetScene()->GetWorldTQS(entity);
		}

		const float maxScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));

		physx::PxSphereGeometry geometry = physx::PxSphereGeometry(maxScale * component.radius);
		myShape = physx::PxRigidActorExt::createExclusiveShape(actor.GetActor(), geometry, *myMaterial);
		myShape->setSimulationFilterData(actor.GetFilterData());
		myShape->setQueryFilterData(actor.GetFilterData());
		myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !component.isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, !component.isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.isTrigger);
		myShape->setLocalPose(PhysXUtilities::ToPhysXTransform(component.offset, glm::vec3{ 0.f }));
		myShape->userData = this;
	}

	SphereColliderShape::~SphereColliderShape()
	{
	}

	void SphereColliderShape::SetRadius(float radius)
	{
		Scene::TQS transform;

		if (myEntity.HasComponent<TransformComponent>())
		{
			transform = myEntity.GetScene()->GetWorldTQS(myEntity);
		}

		const float maxScale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));

		physx::PxSphereGeometry geometry = physx::PxSphereGeometry(maxScale * radius);
		myShape->setGeometry(geometry);

		myEntity.GetComponent<SphereColliderComponent>().radius = radius;
	}

	void SphereColliderShape::SetOffset(const glm::vec3& offset)
	{
		myShape->setLocalPose(PhysXUtilities::ToPhysXTransform(offset, glm::vec3{ 0.f }));
		myEntity.GetComponent<SphereColliderComponent>().offset = offset;
	}

	void SphereColliderShape::SetTrigger(bool isTrigger)
	{
		myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, !isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

		myEntity.GetComponent<SphereColliderComponent>().isTrigger = isTrigger;
	}

	void SphereColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		myShape->setSimulationFilterData(filterData);
		myShape->setQueryFilterData(filterData);
	}

	void SphereColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		VT_CORE_ASSERT(actor, "Actor is null!");
		VT_CORE_ASSERT(myShape, "Shape is null!");
		actor->detachShape(*myShape);
	}

	///// Capsule Collider /////
	CapsuleColliderShape::CapsuleColliderShape(CapsuleColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::Capsule, entity)
	{
		component.added = true;

		SetMaterial(nullptr); // #TODO: Implement actual materials

		Scene::TQS transform;

		if (entity.HasComponent<TransformComponent>())
		{
			transform = entity.GetScene()->GetWorldTQS(entity);
		}

		const float radiusScale = glm::max(transform.scale.x, transform.scale.z);
		const float heightScale = transform.scale.y;

		physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(component.radius * radiusScale, (component.height / 2.f) * heightScale);

		myShape = physx::PxRigidActorExt::createExclusiveShape(actor.GetActor(), geometry, *myMaterial);
		myShape->setSimulationFilterData(actor.GetFilterData());
		myShape->setQueryFilterData(actor.GetFilterData());
		myShape->setQueryFilterData(actor.GetFilterData());
		myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !component.isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, !component.isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.isTrigger);
		myShape->setLocalPose(PhysXUtilities::ToPhysXTransform(component.offset, glm::vec3{ 0.f, glm::pi<float>() / 2.f, 0.f }));
		myShape->userData = this;
	}

	CapsuleColliderShape::~CapsuleColliderShape()
	{
	}

	void CapsuleColliderShape::SetHeight(float height)
	{
		Scene::TQS transform;

		if (myEntity.HasComponent<TransformComponent>())
		{
			transform = myEntity.GetScene()->GetWorldTQS(myEntity);
		}

		const float heightScale = transform.scale.y;

		physx::PxCapsuleGeometry oldGeom;
		myShape->getCapsuleGeometry(oldGeom);

		physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(oldGeom.radius, (height / 2.f) * heightScale);
		myShape->setGeometry(geometry);

		myEntity.GetComponent<CapsuleColliderComponent>().height = height;
	}

	void CapsuleColliderShape::SetRadius(float radius)
	{
		Scene::TQS transform;

		if (myEntity.HasComponent<TransformComponent>())
		{
			transform = myEntity.GetScene()->GetWorldTQS(myEntity);
		}

		const float radiusScale = glm::max(transform.scale.x, transform.scale.z);

		physx::PxCapsuleGeometry oldGeom;
		myShape->getCapsuleGeometry(oldGeom);

		physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(radius * radiusScale, oldGeom.halfHeight);
		myShape->setGeometry(geometry);

		myEntity.GetComponent<CapsuleColliderComponent>().radius = radius;
	}

	void CapsuleColliderShape::SetOffset(const glm::vec3& offset)
	{
		myShape->setLocalPose(PhysXUtilities::ToPhysXTransform(offset, glm::quat{ glm::vec3{ 0.f } }));
		myEntity.GetComponent<CapsuleColliderComponent>().offset = offset;
	}

	void CapsuleColliderShape::SetTrigger(bool isTrigger)
	{
		myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, !isTrigger);
		myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

		myEntity.GetComponent<CapsuleColliderComponent>().isTrigger = isTrigger;
	}

	void CapsuleColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		myShape->setSimulationFilterData(filterData);
		myShape->setQueryFilterData(filterData);
	}

	void CapsuleColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		VT_CORE_ASSERT(actor, "Actor is null!");
		VT_CORE_ASSERT(myShape, "Shape is null!");
		actor->detachShape(*myShape);
	}

	///// Convex Mesh Collider /////
	ConvexMeshShape::ConvexMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::ConvexMesh, entity)
	{
		component.added = true;

		Ref<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(component.material);
		SetMaterial(material);

		Scene::TQS transform;

		if (entity.HasComponent<TransformComponent>())
		{
			transform = entity.GetScene()->GetWorldTQS(entity);
		}

		const std::string colliderName = AssetManager::Get().GetFilePathFromAssetHandle(component.colliderMesh).stem().string() + std::to_string(component.colliderMesh) + "Convex";
		std::vector<MeshColliderData> meshColliderData;
		CookingResult result = CookingResult::Failure;

		if (MeshColliderCache::IsCached(colliderName))
		{
			std::vector<MeshColliderData> cachedData = MeshColliderCache::Get(colliderName).colliderData;
			if (component.subMeshIndex != -1)
			{
				meshColliderData.emplace_back(cachedData.at(component.subMeshIndex));
			}
			else
			{
				meshColliderData = cachedData;
			}

			result = CookingResult::Success;
		}
		else
		{
			result = CookingFactory::CookMesh(component, false, meshColliderData);
		}

		if (!Application::Get().IsRuntime())
		{
			CookingFactory::GenerateDebugMesh(component, meshColliderData);
		}

		if (result == CookingResult::Success)
		{
			for (const auto& colliderData : meshColliderData)
			{
				physx::PxDefaultMemoryInputData input{ colliderData.data.As<physx::PxU8>(), (uint32_t)colliderData.data.GetSize() };
				physx::PxConvexMesh* convexMesh = PhysXInternal::GetPhysXSDK().createConvexMesh(input);

				if (convexMesh)
				{
					physx::PxConvexMeshGeometry convexGeometry = physx::PxConvexMeshGeometry(convexMesh, physx::PxMeshScale(PhysXUtilities::ToPhysXVector(entity.GetScale())));
					convexGeometry.meshFlags = physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;

					physx::PxShape* shape = PhysXInternal::GetPhysXSDK().createShape(convexGeometry, *myMaterial, true);
					shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true); // add collision complexity stuff
					shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.isTrigger);
					shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
					shape->setLocalPose(PhysXUtilities::ToPhysXTransform(colliderData.transform));
					shape->userData = this;
					actor.GetActor().attachShape(*shape);

					myShapes.emplace_back(shape);
					convexMesh->release();
				}
				else
				{
					myMaterial->release();
					VT_CORE_ERROR("Failed to create Convex shape!");
				}
			}
		}
		else
		{
			myMaterial->release();
			VT_CORE_ERROR("Failed to cook Convex shape!");
		}
	}

	ConvexMeshShape::~ConvexMeshShape()
	{
	}

	void ConvexMeshShape::SetOffset(const glm::vec3&)
	{
	}

	void ConvexMeshShape::SetTrigger(bool)
	{
	}

	void ConvexMeshShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		for (auto shape : myShapes)
		{
			shape->setSimulationFilterData(filterData);
			shape->setQueryFilterData(filterData);
		}
	}

	void ConvexMeshShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		for (auto shape : myShapes)
		{
			actor->detachShape(*shape);
		}

		myShapes.clear();
	}

	///// Triangle Mesh Collider /////
	TriangleMeshShape::TriangleMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::TriangleMesh, entity)
	{
		component.added = true;

		Ref<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(component.material);
		SetMaterial(material);

		Scene::TQS transform;

		if (entity.HasComponent<TransformComponent>())
		{
			transform = entity.GetScene()->GetWorldTQS(entity);
		}

		const std::string colliderName = AssetManager::Get().GetFilePathFromAssetHandle(component.colliderMesh).stem().string() + std::to_string(component.colliderMesh) + "Triangle";
		std::vector<MeshColliderData> meshColliderData;
		CookingResult result = CookingResult::Failure;

		if (MeshColliderCache::IsCached(colliderName))
		{
			std::vector<MeshColliderData> cachedData = MeshColliderCache::Get(colliderName).colliderData;
			if (component.subMeshIndex != -1)
			{
				meshColliderData.emplace_back(cachedData.at(component.subMeshIndex));
			}
			else
			{
				meshColliderData = cachedData;
			}

			result = CookingResult::Success;
		}
		else
		{
			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(component.colliderMesh);
			if (mesh && mesh->IsValid())
			{
				result = CookingFactory::CookMesh(component, false, meshColliderData);
			}
			else
			{
				VT_CORE_ERROR("Unable to cook mesh with handle {0}, it is not valid!", component.colliderMesh);
			}
		}

		if (!Application::Get().IsRuntime())
		{
			CookingFactory::GenerateDebugMesh(component, meshColliderData);
		}

		if (result == CookingResult::Success)
		{
			for (const auto& colliderData : meshColliderData)
			{
				physx::PxDefaultMemoryInputData input{ colliderData.data.As<physx::PxU8>(), (uint32_t)colliderData.data.GetSize() };
				physx::PxTriangleMesh* triangleMesh = PhysXInternal::GetPhysXSDK().createTriangleMesh(input);

				if (triangleMesh)
				{
					physx::PxTriangleMeshGeometry triangleGeometry = physx::PxTriangleMeshGeometry(triangleMesh, physx::PxMeshScale(PhysXUtilities::ToPhysXVector(entity.GetScale())));

					physx::PxShape* shape = PhysXInternal::GetPhysXSDK().createShape(triangleGeometry, *myMaterial, true);
					shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true); // add collision complexity stuff
					shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
					shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
					shape->setLocalPose(PhysXUtilities::ToPhysXTransform(colliderData.transform));
					shape->userData = this;
					actor.GetActor().attachShape(*shape);

					myShapes.emplace_back(shape);
					triangleMesh->release();
				}
				else
				{
					myMaterial->release();
					VT_CORE_ERROR("Failed to create Triangle shape!");
				}
			}
		}
		else
		{
			myMaterial->release();
			VT_CORE_ERROR("Failed to cook Triangle shape!");
		}
	}

	TriangleMeshShape::~TriangleMeshShape()
	{
	}

	void TriangleMeshShape::SetOffset(const glm::vec3&)
	{
	}

	void TriangleMeshShape::SetTrigger(bool)
	{
	}

	void TriangleMeshShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		for (auto shape : myShapes)
		{
			shape->setSimulationFilterData(filterData);
			shape->setQueryFilterData(filterData);
		}
	}

	void TriangleMeshShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		for (auto shape : myShapes)
		{
			actor->detachShape(*shape);
		}

		myShapes.clear();
	}
}
