#include "vtpch.h"
#include "PhysicsSystem.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Components/PhysicsComponents.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"
#include "Volt/Physics/PhysicsShapes.h"
#include "Volt/Physics/PhysicsMaterial.h"

namespace Volt
{
	PhysicsSystem::PhysicsSystem(Scene* aScene)
		: myScene(aScene)
	{}

	void PhysicsSystem::Update(float)
	{
		VT_PROFILE_FUNCTION();

		auto& registry = myScene->GetRegistry();

		registry.ForEach<RigidbodyComponent>([this](Wire::EntityId id, RigidbodyComponent& rigidBody)
			{
				if (rigidBody.bodyType != rigidBody.lastRigidbodyData.bodyType)
				{
					auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
					if (actor)
					{
						Physics::GetScene()->RemoveActor(actor);
					}

					Physics::GetScene()->CreateActor(Entity{ id, myScene });
					rigidBody.lastRigidbodyData.bodyType = rigidBody.bodyType;
				}

				if (rigidBody.layerId != rigidBody.lastRigidbodyData.layerId)
				{
					auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
					actor->SetSimulationData(rigidBody.layerId);
					rigidBody.lastRigidbodyData.layerId = rigidBody.layerId;
				}

				if (rigidBody.mass != rigidBody.lastRigidbodyData.mass)
				{
					auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
					actor->SetMass(rigidBody.mass);
					rigidBody.lastRigidbodyData.mass = rigidBody.mass;
				}

				if (rigidBody.linearDrag != rigidBody.lastRigidbodyData.linearDrag)
				{
					auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
					actor->SetLinearDrag(rigidBody.linearDrag);
					rigidBody.lastRigidbodyData.linearDrag = rigidBody.linearDrag;
				}

				if (rigidBody.angularDrag != rigidBody.lastRigidbodyData.angularDrag)
				{
					auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
					actor->SetAngularDrag(rigidBody.angularDrag);
					rigidBody.lastRigidbodyData.angularDrag = rigidBody.angularDrag;
				}

				if (rigidBody.disableGravity != rigidBody.lastRigidbodyData.disableGravity)
				{
					auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
					actor->SetGravityDisabled(rigidBody.disableGravity);
					rigidBody.lastRigidbodyData.disableGravity = rigidBody.disableGravity;
				}

				if (rigidBody.isKinematic != rigidBody.lastRigidbodyData.isKinematic)
				{
					auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
					actor->SetKinematic(rigidBody.isKinematic);
					rigidBody.lastRigidbodyData.isKinematic = rigidBody.isKinematic;
				}

				if (rigidBody.lockFlags != rigidBody.lastRigidbodyData.lockFlags)
				{
					auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
					actor->SetLockFlags(rigidBody.lockFlags);
					rigidBody.lastRigidbodyData.lockFlags = rigidBody.lockFlags;
				}

				// Collision detection?
				// Lock flags?
			});

		registry.ForEach<BoxColliderComponent>([&](Wire::EntityId id, BoxColliderComponent& colliderComp)
			{
				auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
				if (!actor)
				{
					return;
				}

				auto collPtr = actor->GetColliderOfType(ColliderType::Box);
				if (collPtr)
				{
					Ref<BoxColliderShape> collider = std::reinterpret_pointer_cast<BoxColliderShape>(collPtr);

					if (colliderComp.halfSize != colliderComp.lastColliderData.halfSize)
					{
						collider->SetHalfSize(colliderComp.halfSize);
						colliderComp.lastColliderData.halfSize = colliderComp.halfSize;
					}

					if (colliderComp.offset != colliderComp.lastColliderData.offset)
					{
						collider->SetOffset(colliderComp.offset);
						colliderComp.lastColliderData.offset = colliderComp.offset;
					}

					if (colliderComp.isTrigger != colliderComp.lastColliderData.isTrigger)
					{
						collider->SetTrigger(colliderComp.isTrigger);
						colliderComp.lastColliderData.isTrigger = colliderComp.isTrigger;
					}

					if (colliderComp.material != colliderComp.lastColliderData.material)
					{
						auto physicsMaterial = Volt::AssetManager::GetAsset<PhysicsMaterial>(colliderComp.material);
						if (physicsMaterial && physicsMaterial->IsValid())
						{
							collider->SetMaterial(physicsMaterial);
							colliderComp.lastColliderData.material = colliderComp.material;
						}
						else
						{
							VT_CORE_WARN("Trying to set invalid physics material!");
							colliderComp.material = colliderComp.lastColliderData.material;
						}
					}
				}
			});

		registry.ForEach<SphereColliderComponent>([&](Wire::EntityId id, SphereColliderComponent& colliderComp)
			{
				auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
				if (!actor)
				{
					return;
				}

				auto collPtr = actor->GetColliderOfType(ColliderType::Sphere);
				if (collPtr)
				{
					Ref<SphereColliderShape> collider = std::reinterpret_pointer_cast<SphereColliderShape>(collPtr);

					if (colliderComp.radius != colliderComp.lastColliderData.radius)
					{
						collider->SetRadius(colliderComp.radius);
						colliderComp.lastColliderData.radius = colliderComp.radius;
					}

					if (colliderComp.offset != colliderComp.lastColliderData.offset)
					{
						collider->SetOffset(colliderComp.offset);
						colliderComp.lastColliderData.offset = colliderComp.offset;
					}

					if (colliderComp.isTrigger != colliderComp.lastColliderData.isTrigger)
					{
						collider->SetTrigger(colliderComp.isTrigger);
						colliderComp.lastColliderData.isTrigger = colliderComp.isTrigger;
					}

					if (colliderComp.material != colliderComp.lastColliderData.material)
					{
						auto physicsMaterial = Volt::AssetManager::GetAsset<PhysicsMaterial>(colliderComp.material);
						if (physicsMaterial && physicsMaterial->IsValid())
						{
							collider->SetMaterial(physicsMaterial);
							colliderComp.lastColliderData.material = colliderComp.material;
						}
						else
						{
							VT_CORE_WARN("Trying to set invalid physics material!");
							colliderComp.material = colliderComp.lastColliderData.material;
						}
					}
				}
			});

		registry.ForEach<CapsuleColliderComponent>([&](Wire::EntityId id, CapsuleColliderComponent& colliderComp)
			{
				auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
				if (!actor)
				{
					return;
				}

				auto collPtr = actor->GetColliderOfType(ColliderType::Capsule);
				if (collPtr)
				{
					Ref<CapsuleColliderShape> collider = std::reinterpret_pointer_cast<CapsuleColliderShape>(collPtr);

					if (colliderComp.radius != colliderComp.lastColliderData.radius)
					{
						collider->SetRadius(colliderComp.radius);
						colliderComp.lastColliderData.radius = colliderComp.radius;
					}

					if (colliderComp.height != colliderComp.lastColliderData.height)
					{
						collider->SetHeight(colliderComp.height);
						colliderComp.lastColliderData.height = colliderComp.height;
					}

					if (colliderComp.offset != colliderComp.lastColliderData.offset)
					{
						collider->SetOffset(colliderComp.offset);
						colliderComp.lastColliderData.offset = colliderComp.offset;
					}

					if (colliderComp.isTrigger != colliderComp.lastColliderData.isTrigger)
					{
						collider->SetTrigger(colliderComp.isTrigger);
						colliderComp.lastColliderData.isTrigger = colliderComp.isTrigger;
					}

					if (colliderComp.material != colliderComp.lastColliderData.material)
					{
						auto physicsMaterial = Volt::AssetManager::GetAsset<PhysicsMaterial>(colliderComp.material);
						if (physicsMaterial && physicsMaterial->IsValid())
						{
							collider->SetMaterial(physicsMaterial);
							colliderComp.lastColliderData.material = colliderComp.material;
						}
						else
						{
							VT_CORE_WARN("Trying to set invalid physics material!");
							colliderComp.material = colliderComp.lastColliderData.material;
						}
					}
				}
			});

		registry.ForEach<MeshColliderComponent>([&](Wire::EntityId id, const MeshColliderComponent&)
			{
				auto actor = Physics::GetScene()->GetActor(Entity{ id, myScene });
				if (!actor)
				{
					return;
				}
			});
	}
}
