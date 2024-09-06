#include "vtpch.h"
#include "Volt/Components/PhysicsComponents.h"

#include "Volt/Scene/SceneManager.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"

namespace Volt
{
	void RigidbodyComponent::OnCreate(RigidbodyComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		Physics::CreateActor(entity);
	}

	void RigidbodyComponent::OnDestroy(RigidbodyComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetActor(entity);
		if (actor)
		{
			Physics::GetScene()->RemoveActor(actor);
		}
	}

	void CharacterControllerComponent::OnCreate(CharacterControllerComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		Physics::CreateControllerActor(entity);
	}

	void CharacterControllerComponent::OnDestroy(CharacterControllerComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetControllerActor(entity);
		if (actor)
		{
			Physics::GetScene()->RemoveControllerActor(actor);
		}
	}

	void BoxColliderComponent::OnCreate(BoxColliderComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetActor(entity);

		if (actor && !component.added)
		{
			actor->AddCollider(component, entity);
		}
	}

	void BoxColliderComponent::OnDestroy(BoxColliderComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetActor(entity);

		if (actor)
		{
			actor->RemoveCollider(ColliderType::Box);
		}
	}

	void SphereColliderComponent::OnCreate(SphereColliderComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetActor(entity);

		if (actor && !component.added)
		{
			actor->AddCollider(component, entity);
		}
	}

	void SphereColliderComponent::OnDestroy(SphereColliderComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetActor(entity);

		if (actor)
		{
			actor->RemoveCollider(ColliderType::Sphere);
		}
	}

	void CapsuleColliderComponent::OnCreate(CapsuleColliderComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetActor(entity);

		if (actor && !component.added)
		{
			actor->AddCollider(component, entity);
		}
	}

	void CapsuleColliderComponent::OnDestroy(CapsuleColliderComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetActor(entity);

		if (actor)
		{
			actor->RemoveCollider(ColliderType::Capsule);
		}
	}

	void MeshColliderComponent::OnCreate(MeshColliderComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetActor(entity);

		if (actor && !component.added)
		{
			actor->AddCollider(component, entity);
		}
	}

	void MeshColliderComponent::OnDestroy(MeshColliderComponent& component, entt::entity id)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity entity{ id, SceneManager::GetActiveScene() };
		auto actor = Physics::GetScene()->GetActor(entity);

		if (actor)
		{
			if (component.isConvex)
			{
				actor->RemoveCollider(ColliderType::ConvexMesh);
			}
			else
			{
				actor->RemoveCollider(ColliderType::TriangleMesh);
			}
		}
	}
}
