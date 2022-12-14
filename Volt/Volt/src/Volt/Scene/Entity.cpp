#include "vtpch.h"
#include "Entity.h"
#include "Volt/Scene/Scene.h"

#include "Volt/Components/Components.h"
#include "Volt/Components/PhysicsComponents.h"
#include "Volt/Scripting/Script.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"
#include "Volt/Core/Profiling.h"

namespace Volt
{
	Entity::Entity()
		: myId(Wire::NullID)
	{}

	Entity::Entity(Wire::EntityId id, Scene* scene)
		: myId(id), myScene(scene)
	{}

	Entity::Entity(const Entity& entity)
	{
		*this = entity;
	}

	Entity::~Entity()
	{}

	bool Entity::HasScript(const std::string& scriptName)
	{
		return ScriptEngine::IsScriptRegistered(ScriptRegistry::GetGUIDFromName(scriptName), myId);
	}

	void Entity::AddScript(const std::string& scriptName)
	{
		if (ScriptEngine::IsScriptRegistered(ScriptRegistry::GetGUIDFromName(scriptName), myId))
		{
			VT_CORE_WARN("Script {0} already registered to entity {1}! Skipping!", scriptName, myId);
			return;
		}

		if (!HasComponent<ScriptComponent>())
		{
			AddComponent<ScriptComponent>();
		}

		auto& scriptComp = GetComponent<ScriptComponent>();

		Ref<Script> scriptInstance = ScriptRegistry::Create(scriptName, Entity{ myId, myScene });
		scriptComp.scripts.emplace_back(scriptInstance->GetGUID());
		ScriptEngine::RegisterToEntity(scriptInstance, myId);

		scriptInstance->OnAwake();
	}

	void Entity::RemoveScript(const std::string& scriptName)
	{
		RemoveScript(ScriptRegistry::GetGUIDFromName(scriptName));
	}

	void Entity::RemoveScript(WireGUID scriptGUID)
	{
		if (!ScriptEngine::IsScriptRegistered(scriptGUID, myId))
		{
			VT_CORE_WARN("Script {0} has not been registered to entity {1}! Skipping!", ScriptRegistry::GetNameFromGUID(scriptGUID), myId);
			return;
		}

		if (!HasComponent<ScriptComponent>())
		{
			return;
		}

		auto& scriptComp = GetComponent<ScriptComponent>();
		auto it = std::find(scriptComp.scripts.begin(), scriptComp.scripts.end(), scriptGUID);
		scriptComp.scripts.erase(it);

		Ref<Script> scriptInstance = ScriptEngine::GetScript(myId, scriptGUID);
		scriptInstance->OnDetach();
		ScriptEngine::UnregisterFromEntity(scriptGUID, myId);
	}

	const std::string Entity::GetTag()
	{
		if (HasComponent<TagComponent>())
		{
			return GetComponent<TagComponent>().tag;
		}

		return {};
	}

	const gem::vec3 Entity::GetPosition() const
	{
		if (myScene->GetRegistry().HasComponent<TransformComponent>(myId))
		{
			const auto& comp = myScene->GetRegistry().GetComponent<TransformComponent>(myId);
			return comp.position;
		}

		return gem::vec3{ 0.f, 0.f, 0.f };
	}

	const gem::vec3 Entity::GetRotation() const
	{
		if (myScene->GetRegistry().HasComponent<TransformComponent>(myId))
		{
			const auto& comp = myScene->GetRegistry().GetComponent<TransformComponent>(myId);
			return comp.rotation;
		}

		return gem::vec3{ 0.f, 0.f, 0.f };
	}

	const gem::vec3 Entity::GetScale() const
	{
		if (myScene->GetRegistry().HasComponent<TransformComponent>(myId))
		{
			const auto& comp = myScene->GetRegistry().GetComponent<TransformComponent>(myId);
			return comp.scale;
		}

		return gem::vec3{ 0.f, 0.f, 0.f };
	}

	const gem::vec3 Entity::GetWorldPosition() const
	{
		auto trs = myScene->GetWorldSpaceTRS(*this);
		return trs.position;
	}

	const gem::vec3 Entity::GetWorldRotation() const
	{
		auto trs = myScene->GetWorldSpaceTRS(*this);
		return trs.rotation;
	}

	const gem::vec3 Entity::GetWorldScale() const
	{
		auto trs = myScene->GetWorldSpaceTRS(*this);
		return trs.scale;
	}

	const std::vector<Volt::Entity> Entity::GetChilden() const
	{
		std::vector<Volt::Entity> children;

		for (auto& id : GetComponent<Volt::RelationshipComponent>().Children)
		{
			children.push_back(Volt::Entity{ id, myScene });
		}

		return children;
	}

	const Volt::Entity Entity::GetParent() const
	{
		return Volt::Entity{ GetComponent<RelationshipComponent>().Parent, myScene };
	}

	const Ref<PhysicsActor> Entity::GetPhysicsActor() const
	{
		return Physics::GetScene()->GetActor(*this);
	}

	void Entity::SetWorldPosition(const gem::vec3& position, bool updatePhysics)
	{
		VT_PROFILE_FUNCTION();

		Volt::Entity parent = GetParent();
		gem::mat4 parentTransform = 1.f;

		if (parent)
		{
			parentTransform = myScene->GetWorldSpaceTransform(parent);
		}

		const gem::mat4 newWorldTransform = gem::translate(gem::mat4{ 1.f }, position);
		const gem::mat4 transform = gem::inverse(parentTransform) * newWorldTransform;

		gem::vec3 t, r, s;
		gem::decompose(transform, t, r, s);

		SetPosition(t, updatePhysics);
	}

	void Entity::SetPosition(const gem::vec3& position, bool updatePhysics)
	{
		myScene->GetRegistry().GetComponent<TransformComponent>(myId).position = position;
		if (myScene->GetRegistry().HasComponent<RigidbodyComponent>(myId) && Physics::GetScene() && updatePhysics)
		{
			auto actor = Physics::GetScene()->GetActor(*this);
			if (actor)
			{
				actor->SetPosition(GetWorldPosition());
			}
		}
	}

	void Entity::SetRotation(const gem::vec3& rotation)
	{
		myScene->GetRegistry().GetComponent<TransformComponent>(myId).rotation = rotation;
		if (myScene->GetRegistry().HasComponent<RigidbodyComponent>(myId))
		{
			auto actor = Physics::GetScene()->GetActor(*this);
			if (actor)
			{
				actor->SetRotation(GetWorldRotation());
			}
		}
	}

	void Entity::SetScale(const gem::vec3& scale)
	{
		myScene->GetRegistry().GetComponent<TransformComponent>(myId).scale = scale;
	}

	const gem::mat4 Entity::GetTransform() const
	{
		if (myScene->GetRegistry().HasComponent<TransformComponent>(myId))
		{
			const auto& comp = myScene->GetRegistry().GetComponent<TransformComponent>(myId);
			return comp.GetTransform();
		}

		return gem::mat4{ 1.f };
	}

	const gem::mat4 Entity::GetWorldTransform() const
	{
		return myScene->GetWorldSpaceTransform(*this);
	}

	const gem::vec3 Entity::GetForward() const
	{
		if (myScene->GetRegistry().HasComponent<TransformComponent>(myId))
		{
			const auto& comp = myScene->GetRegistry().GetComponent<TransformComponent>(myId);
			return comp.GetForward();
		}

		return gem::vec3{ 0.f, 0.f, 1.f };
	}

	const gem::vec3 Entity::GetRight() const
	{
		if (myScene->GetRegistry().HasComponent<TransformComponent>(myId))
		{
			const auto& comp = myScene->GetRegistry().GetComponent<TransformComponent>(myId);
			return comp.GetRight();
		}

		return gem::vec3{ 1.f, 0.f, 0.f };
	}

	const gem::vec3 Entity::GetUp() const
	{
		if (myScene->GetRegistry().HasComponent<TransformComponent>(myId))
		{
			const auto& comp = myScene->GetRegistry().GetComponent<TransformComponent>(myId);
			return comp.GetUp();
		}

		return gem::vec3{ 0.f, 1.f, 0.f };
	}

	const gem::vec3 Entity::GetWorldForward() const
	{
		return myScene->GetWorldForward(*this);
	}

	const gem::vec3 Entity::GetWorldRight() const
	{
		return myScene->GetWorldRight(*this);
	}

	const gem::vec3 Entity::GetWorldUp() const
	{
		return myScene->GetWorldUp(*this);
	}

	Volt::Entity& Entity::operator=(const Entity& entity)
	{
		myId = entity.myId;
		myScene = entity.myScene;
		return *this;
	}

	void Entity::Copy(Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity, std::vector<WireGUID> aExcludedComponents, bool aShouldReset)
	{
		// Remove all existing components (except excluded)
		if (aShouldReset)
		{
			for (const auto& [guid, pool] : aTargetRegistry.GetPools())
			{
				if (std::find(aExcludedComponents.begin(), aExcludedComponents.end(), guid) != aExcludedComponents.end())
				{
					continue;
				}

				if (pool->HasComponent(aTargetEntity))
				{
					pool->RemoveComponent(aTargetEntity);
				}
			}
		}

		for (const auto& [guid, pool] : aSrcRegistry.GetPools())
		{
			if (std::find(aExcludedComponents.begin(), aExcludedComponents.end(), guid) != aExcludedComponents.end())
			{
				continue;
			}

			if (guid == ScriptComponent::comp_guid && pool->HasComponent(aSrcEntity))
			{
				if (!aTargetRegistry.HasComponent(guid, aTargetEntity))
				{
					aTargetRegistry.AddComponent(guid, aTargetEntity);
				}

				ScriptComponent* otherComponent = (ScriptComponent*)aTargetRegistry.GetComponentPtr(guid, aTargetEntity);
				ScriptComponent* scriptComp = (ScriptComponent*)pool->GetComponent(aSrcEntity);

				otherComponent->scripts = scriptComp->scripts;
				continue;
			}

			if (pool->HasComponent(aSrcEntity))
			{
				if (!aTargetRegistry.HasComponent(guid, aTargetEntity))
				{
					aTargetRegistry.AddComponent(guid, aTargetEntity);
				}

				const uint8_t* thisComponent = (uint8_t*)pool->GetComponent(aSrcEntity);
				uint8_t* otherComponent = (uint8_t*)aTargetRegistry.GetComponentPtr(guid, aTargetEntity);

				const auto& compInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);
				for (const auto& prop : compInfo.properties)
				{
					if (prop.serializable)
					{
						switch (prop.type)
						{
							case Wire::ComponentRegistry::PropertyType::Bool: (*(bool*)&otherComponent[prop.offset]) = (*(bool*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Int: (*(int32_t*)&otherComponent[prop.offset]) = (*(int32_t*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::UInt: (*(uint32_t*)&otherComponent[prop.offset]) = (*(uint32_t*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Short: (*(int16_t*)&otherComponent[prop.offset]) = (*(int16_t*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::UShort: (*(uint16_t*)&otherComponent[prop.offset]) = (*(uint16_t*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Char: (*(int8_t*)&otherComponent[prop.offset]) = (*(int8_t*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::UChar: (*(uint8_t*)&otherComponent[prop.offset]) = (*(uint8_t*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Float: (*(float*)&otherComponent[prop.offset]) = (*(float*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Double: (*(double*)&otherComponent[prop.offset]) = (*(double*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Vector2: (*(gem::vec2*)&otherComponent[prop.offset]) = (*(gem::vec2*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Vector3: (*(gem::vec3*)&otherComponent[prop.offset]) = (*(gem::vec3*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Vector4: (*(gem::vec4*)&otherComponent[prop.offset]) = (*(gem::vec4*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::String: (*(std::string*)&otherComponent[prop.offset]) = (*(std::string*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Int64: (*(int64_t*)&otherComponent[prop.offset]) = (*(int64_t*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::UInt64: (*(uint64_t*)&otherComponent[prop.offset]) = (*(uint64_t*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::AssetHandle: (*(AssetHandle*)&otherComponent[prop.offset]) = (*(AssetHandle*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Color3: (*(gem::vec3*)&otherComponent[prop.offset]) = (*(gem::vec3*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Color4: (*(gem::vec4*)&otherComponent[prop.offset]) = (*(gem::vec4*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Folder: (*(std::filesystem::path*)&otherComponent[prop.offset]) = (*(std::filesystem::path*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Path: (*(std::filesystem::path*)&otherComponent[prop.offset]) = (*(std::filesystem::path*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::EntityId: (*(Wire::EntityId*)&otherComponent[prop.offset]) = (*(Wire::EntityId*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::GUID: (*(WireGUID*)&otherComponent[prop.offset]) = (*(WireGUID*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Enum: (*(uint32_t*)&otherComponent[prop.offset]) = (*(uint32_t*)&thisComponent[prop.offset]); break;
							case Wire::ComponentRegistry::PropertyType::Vector:
							{
								switch (prop.vectorType)
								{
									case Wire::ComponentRegistry::PropertyType::Bool: (*(std::vector<bool>*) & otherComponent[prop.offset]) = (*(std::vector<bool>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Int: (*(std::vector<int32_t>*) & otherComponent[prop.offset]) = (*(std::vector<int32_t>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::UInt: (*(std::vector<uint32_t>*) & otherComponent[prop.offset]) = (*(std::vector<uint32_t>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Short: (*(std::vector<int16_t>*) & otherComponent[prop.offset]) = (*(std::vector<int16_t>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::UShort: (*(std::vector<uint16_t>*) & otherComponent[prop.offset]) = (*(std::vector<uint16_t>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Char: (*(std::vector<int8_t>*) & otherComponent[prop.offset]) = (*(std::vector<int8_t>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::UChar: (*(std::vector<uint8_t>*) & otherComponent[prop.offset]) = (*(std::vector<uint8_t>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Float: (*(std::vector<float>*) & otherComponent[prop.offset]) = (*(std::vector<float>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Double: (*(std::vector<double>*) & otherComponent[prop.offset]) = (*(std::vector<double>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Vector2: (*(std::vector<gem::vec2>*) & otherComponent[prop.offset]) = (*(std::vector<gem::vec2>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Vector3: (*(std::vector<gem::vec3>*) & otherComponent[prop.offset]) = (*(std::vector<gem::vec3>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Vector4: (*(std::vector<gem::vec4>*) & otherComponent[prop.offset]) = (*(std::vector<gem::vec4>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::String: (*(std::vector<std::string>*) & otherComponent[prop.offset]) = (*(std::vector<std::string>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Int64: (*(std::vector<int64_t>*) & otherComponent[prop.offset]) = (*(std::vector<int64_t>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::UInt64: (*(std::vector<uint64_t>*) & otherComponent[prop.offset]) = (*(std::vector<uint64_t>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::AssetHandle: (*(std::vector<AssetHandle>*) & otherComponent[prop.offset]) = (*(std::vector<AssetHandle>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Color4: (*(std::vector<gem::vec2>*) & otherComponent[prop.offset]) = (*(std::vector<gem::vec2>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Color3: (*(std::vector<gem::vec3>*) & otherComponent[prop.offset]) = (*(std::vector<gem::vec3>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Folder: (*(std::vector<gem::vec4>*) & otherComponent[prop.offset]) = (*(std::vector<gem::vec4>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::EntityId: (*(std::vector<Wire::EntityId>*) & otherComponent[prop.offset]) = (*(std::vector<Wire::EntityId>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::GUID: (*(std::vector<WireGUID>*) & otherComponent[prop.offset]) = (*(std::vector<WireGUID>*) & thisComponent[prop.offset]); break;
									case Wire::ComponentRegistry::PropertyType::Enum: (*(uint32_t*)&otherComponent[prop.offset]) = (*(uint32_t*)&thisComponent[prop.offset]); break;
								}

								break;
							}
						}
					}
				}
			}
		}
	}

	Wire::EntityId Entity::Duplicate(Wire::Registry& aRegistry, Wire::EntityId aSrcEntity)
	{
		Wire::EntityId parent = Wire::NullID;

		if (aRegistry.HasComponent<RelationshipComponent>(aSrcEntity))
		{
			auto& relComp = aRegistry.GetComponent<RelationshipComponent>(aSrcEntity);
			parent = relComp.Parent;
		}

		Wire::EntityId newEnt = DuplicateInternal(aRegistry, aSrcEntity, parent);

		if (parent != Wire::NullID)
		{
			auto& relComp = aRegistry.GetComponent<RelationshipComponent>(parent);
			relComp.Children.emplace_back(newEnt);
		}

		return newEnt;
	}

	Wire::EntityId Entity::DuplicateInternal(Wire::Registry& aRegistry, Wire::EntityId aSrcEntity, Wire::EntityId aParent)
	{
		Wire::EntityId newEnt = aRegistry.CreateEntity();
		aRegistry.AddComponent<RelationshipComponent>(newEnt);

		Copy(aRegistry, aRegistry, aSrcEntity, newEnt, { Wire::ComponentRegistry::GetRegistryDataFromName("RelationshipComponent").guid });

		if (aRegistry.HasComponent<RelationshipComponent>(aSrcEntity))
		{
			auto& relComp = aRegistry.GetComponent<RelationshipComponent>(aSrcEntity);
			std::vector<Wire::EntityId> newChildren;

			for (const auto& child : relComp.Children)
			{
				newChildren.emplace_back(DuplicateInternal(aRegistry, child, newEnt));
			}

			auto& newRelComp = aRegistry.GetComponent<RelationshipComponent>(newEnt);
			newRelComp.Children = newChildren;
			newRelComp.Parent = aParent;
		}

		return newEnt;
	}
}