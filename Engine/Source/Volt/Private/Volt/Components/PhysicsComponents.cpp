#include "vtpch.h"
#include "Volt/Components/PhysicsComponents.h"

#include "Volt/Scene/SceneManager.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"

namespace Volt
{
	void RigidbodyComponent::OnCreate(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		Physics::CreateActor(sceneEntity);
	}

	void RigidbodyComponent::OnDestroy(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);
		if (actor)
		{
			Physics::GetScene()->RemoveActor(actor);
		}
	}

	void RigidbodyComponent::OnTransformChanged(PhysicsTransformEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);
		if (actor)
		{
			actor->SetPosition(entity.GetPosition(), true, false);
			actor->SetRotation(entity.GetRotation(), true, false);
		}
	}

	void CharacterControllerComponent::OnCreate(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		Physics::CreateControllerActor(sceneEntity);
	}

	void CharacterControllerComponent::OnDestroy(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetControllerActor(sceneEntity);
		if (actor)
		{
			Physics::GetScene()->RemoveControllerActor(actor);
		}
	}

	void CharacterControllerComponent::OnTransformChanged(PhysicsTransformEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetControllerActor(sceneEntity);
		if (actor)
		{
			actor->SetFootPosition(entity.GetPosition());
		}
	}

	void BoxColliderComponent::OnCreate(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto& component = entity.GetComponent<BoxColliderComponent>();

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);

		if (actor && !component.added)
		{
			actor->AddCollider(component, sceneEntity);
		}
	}

	void BoxColliderComponent::OnDestroy(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);

		if (actor)
		{
			actor->RemoveCollider(ColliderType::Box);
		}
	}

	void SphereColliderComponent::OnCreate(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto& component = entity.GetComponent<SphereColliderComponent>();

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);

		if (actor && !component.added)
		{
			actor->AddCollider(component, sceneEntity);
		}
	}

	void SphereColliderComponent::OnDestroy(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);

		if (actor)
		{
			actor->RemoveCollider(ColliderType::Sphere);
		}
	}

	void CapsuleColliderComponent::OnCreate(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto& component = entity.GetComponent<CapsuleColliderComponent>();

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);

		if (actor && !component.added)
		{
			actor->AddCollider(component, sceneEntity);
		}
	}

	void CapsuleColliderComponent::OnDestroy(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);

		if (actor)
		{
			actor->RemoveCollider(ColliderType::Capsule);
		}
	}

	void MeshColliderComponent::OnCreate(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto& component = entity.GetComponent<MeshColliderComponent>();

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);

		if (actor && !component.added)
		{
			actor->AddCollider(component, sceneEntity);
		}
	}

	void MeshColliderComponent::OnDestroy(PhysicsEntity entity)
	{
		if (!SceneManager::IsPlaying())
		{
			return;
		}

		auto& component = entity.GetComponent<MeshColliderComponent>();

		Entity sceneEntity = SceneManager::GetActiveScene()->GetSceneEntityFromScriptingEntity(entity);
		auto actor = Physics::GetScene()->GetActor(sceneEntity);

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
