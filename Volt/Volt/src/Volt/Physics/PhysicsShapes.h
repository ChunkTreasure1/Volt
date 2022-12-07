#pragma once

#include "Volt/Physics/PhysicsEnums.h"
#include "Volt/Scene/Entity.h"
#include "Volt/Components/PhysicsComponents.h"

#include <GEM/gem.h>
#include <PhysX/PxPhysicsAPI.h>

namespace Volt
{
	class PhysicsMaterial;
	class ColliderShape
	{
	public:
		virtual ~ColliderShape() {}

		void Release();
		void SetMaterial(Ref<PhysicsMaterial> material);

		virtual const bool IsTrigger() const = 0;
		virtual void SetTrigger(bool isTrigger) const = 0;

		virtual const gem::vec3& GetOffset() const = 0;
		virtual void SetOffset(const gem::vec3& offset) = 0;

		virtual void SetFilterData(const physx::PxFilterData& filterData) = 0;
		virtual void DetachFromActor(physx::PxRigidActor* actor) = 0;
		inline ColliderType GetType() const { return myColliderType; }

		inline physx::PxMaterial& GetMaterial() const { return *myMaterial; }
		inline const bool IsValid() const { return myMaterial != nullptr; }

	protected:
		ColliderShape(ColliderType type, Entity entity);
	
		ColliderType myColliderType = ColliderType::Box;
		Entity myEntity;

		physx::PxMaterial* myMaterial = nullptr;
	};

	class PhysicsActor;
	class BoxColliderShape : public ColliderShape
	{
	public:
		BoxColliderShape(BoxColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~BoxColliderShape() override;

		inline const gem::vec3& GetHalfSize() const { return myEntity.GetComponent<BoxColliderComponent>().halfSize; }
		void SetHalfSize(const gem::vec3& halfSize);

		inline const bool IsTrigger() const override { return myEntity.GetComponent<BoxColliderComponent>().isTrigger; }
		void SetTrigger(bool isTrigger) const override;

		inline const gem::vec3& GetOffset() const override { return myEntity.GetComponent<BoxColliderComponent>().offset; }
		void SetOffset(const gem::vec3& offset) override;
	
		void SetFilterData(const physx::PxFilterData& filterData) override;
		void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		physx::PxShape* myShape;
	};

	class SphereColliderShape : public ColliderShape
	{
	public:
		SphereColliderShape(SphereColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~SphereColliderShape() override;

		inline const float GetRadius() const { return myEntity.GetComponent<SphereColliderComponent>().radius; }
		void SetRadius(float radius);

		const gem::vec3& GetOffset() const override { return myEntity.GetComponent<SphereColliderComponent>().offset; }
		void SetOffset(const gem::vec3& offset) override;

		inline const bool IsTrigger() const override { return myEntity.GetComponent<SphereColliderComponent>().isTrigger; }
		void SetTrigger(bool isTrigger) const override;

		void SetFilterData(const physx::PxFilterData& filterData) override;
		void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		physx::PxShape* myShape;
	};

	class CapsuleColliderShape : public ColliderShape
	{
	public:
		CapsuleColliderShape(CapsuleColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~CapsuleColliderShape() override;

		inline const float GetHeight() const { return myEntity.GetComponent<CapsuleColliderComponent>().height; }
		void SetHeight(float height);

		inline const float GetRadius() const { return myEntity.GetComponent<CapsuleColliderComponent>().radius; }
		void SetRadius(float radius);

		const gem::vec3& GetOffset() const override { return myEntity.GetComponent<CapsuleColliderComponent>().offset; }
		void SetOffset(const gem::vec3& offset) override;

		inline const bool IsTrigger() const override { return myEntity.GetComponent<CapsuleColliderComponent>().isTrigger; }
		void SetTrigger(bool isTrigger) const override;

		void SetFilterData(const physx::PxFilterData& filterData) override;
		void DetachFromActor(physx::PxRigidActor* actor) override;
	
	private:
		physx::PxShape* myShape;
	};

	class ConvexMeshShape : public ColliderShape
	{
	public:
		ConvexMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~ConvexMeshShape() override;

		const gem::vec3& GetOffset() const override { return gem::vec3{ 0.f, 0.f, 0.f }; }
		void SetOffset(const gem::vec3& offset) override;

		inline const bool IsTrigger() const override { return myEntity.GetComponent<MeshColliderComponent>().isTrigger; }
		void SetTrigger(bool isTrigger) const override;

		void SetFilterData(const physx::PxFilterData& filterData) override;
		void DetachFromActor(physx::PxRigidActor* actor) override;
	
	private:
		std::vector<physx::PxShape*> myShapes;
	};

	class TriangleMeshShape : public ColliderShape
	{
	public:
		TriangleMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~TriangleMeshShape() override;

		const gem::vec3& GetOffset() const override { return gem::vec3{ 0.f, 0.f, 0.f }; }
		void SetOffset(const gem::vec3& offset) override;

		inline const bool IsTrigger() const override { return myEntity.GetComponent<MeshColliderComponent>().isTrigger; }
		void SetTrigger(bool isTrigger) const override;

		void SetFilterData(const physx::PxFilterData& filterData) override;
		void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		std::vector<physx::PxShape*> myShapes;
	};
}