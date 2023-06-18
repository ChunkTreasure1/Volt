#include "vtpch.h"
#include "Entity.h"
#include "Volt/Scene/Scene.h"

#include "Volt/Components/Components.h"
#include "Volt/Components/PhysicsComponents.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"
#include "Volt/Scripting/Mono/MonoScriptClass.h"

#include <GraphKey/Graph.h>
#include "Volt/Net/SceneInteraction/NetActorComponent.h"

#include "EnTTTesting.h"

namespace Volt
{
	Entity::Entity()
		: myId(Wire::NullID)
	{
	}

	Entity::Entity(Wire::EntityId id, Scene* scene)
		: myId(id), myScene(scene)
	{
	}

	Entity::Entity(const Entity& entity)
	{
		*this = entity;
	}

	Entity::~Entity()
	{
	}

	const std::string Entity::GetTag()
	{
		if (HasComponent<TagComponent>())
		{
			return GetComponent<TagComponent>().tag;
		}

		return {};
	}

	void Entity::SetTag(const std::string& tag)
	{
		if (HasComponent<TagComponent>())
		{
			GetComponent<TagComponent>().tag = tag;
		}
	}

	const glm::vec3 Entity::GetLocalPosition() const
	{
		if (HasComponent<TransformComponent>())
		{
			const auto& comp = GetComponent<TransformComponent>();
			return comp.position;
		}

		return glm::vec3{ 0.f, 0.f, 0.f };
	}

	const glm::quat Entity::GetLocalRotation() const
	{
		if (HasComponent<TransformComponent>())
		{
			const auto& comp = GetComponent<TransformComponent>();
			return comp.rotation;
		}

		return glm::vec3{ 0.f, 0.f, 0.f };
	}

	const glm::vec3 Entity::GetLocalScale() const
	{
		if (HasComponent<TransformComponent>())
		{
			const auto& comp = GetComponent<TransformComponent>();
			return comp.scale;
		}

		return glm::vec3{ 0.f, 0.f, 0.f };
	}

	const glm::vec3 Entity::GetPosition() const
	{
		auto trs = myScene->GetWorldSpaceTRS(*this);
		return trs.position;
	}

	const glm::quat Entity::GetRotation() const
	{
		auto trs = myScene->GetWorldSpaceTRS(*this);
		return trs.rotation;
	}

	const glm::vec3 Entity::GetScale() const
	{
		auto trs = myScene->GetWorldSpaceTRS(*this);
		return trs.scale;
	}

	const bool Entity::IsVisible() const
	{
		if (!HasComponent<TransformComponent>())
		{
			return false;
		}

		return GetComponent<TransformComponent>().visible;
	}

	void Entity::SetVisible(bool state)
	{
		if (!HasComponent<TransformComponent>())
		{
			return;
		}

		bool lastVisibleState = GetComponent<TransformComponent>().visible;
		GetComponent<TransformComponent>().visible = state;

		for (auto child : GetChilden())
		{
			child.SetVisible(state);
		}

		if (lastVisibleState == state)
		{
			return;
		}

		if (MonoScriptEngine::IsRunning() && HasComponent<MonoScriptComponent>())
		{
			const auto& monoComp = GetComponent<MonoScriptComponent>();
			for (const auto& scriptId : monoComp.scriptIds)
			{
				if (state)
				{
					MonoScriptEngine::OnEnableInstance(scriptId);
				}
				else
				{
					MonoScriptEngine::OnDisableInstance(scriptId);
				}
			}
		}
	}

	void Entity::SetLocked(bool state)
	{
		if (!HasComponent<TransformComponent>())
		{
			return;
		}

		bool lastLockedState = GetComponent<TransformComponent>().locked;
		GetComponent<TransformComponent>().locked = state;

		for (auto child : GetChilden())
		{
			child.SetLocked(state);
		}
	}

	const uint32_t Entity::GetLayerId() const
	{
		if (!HasComponent<EntityDataComponent>())
		{
			return 0;
		}

		return GetComponent<EntityDataComponent>().layerId;
	}

	const std::vector<Volt::Entity> Entity::GetChilden() const
	{
		std::vector<Volt::Entity> children;

		if (!HasComponent<Volt::RelationshipComponent>())
		{
			return children;
		}

		for (auto& id : GetComponent<Volt::RelationshipComponent>().Children)
		{
			children.emplace_back(id, myScene);
		}

		return children;
	}

	const Volt::Entity Entity::GetParent()
	{
		if (!HasComponent<RelationshipComponent>())
		{
			AddComponent<RelationshipComponent>();
		}

		return Volt::Entity{ GetComponent<RelationshipComponent>().Parent, myScene };
	}

	void Entity::ResetParent()
	{
		if (!HasComponent<RelationshipComponent>())
		{
			return;
		}

		auto parent = GetParent();
		parent.RemoveChild(*this);
		GetComponent<RelationshipComponent>().Parent = Wire::NullID;
	}

	void Entity::ResetChildren()
	{
		auto& children = GetComponent<RelationshipComponent>().Children;

		for (const auto& child : children)
		{
			Entity childEnt{ child, myScene };
			if (childEnt.GetParent().GetId() == myId)
			{
				childEnt.GetComponent<RelationshipComponent>().Parent = Wire::NullID;
			}
		}

		children.clear();
	}

	void Entity::RemoveChild(Volt::Entity entity)
	{
		if (!HasComponent<RelationshipComponent>())
		{
			return;
		}

		auto& relComp = GetComponent<RelationshipComponent>();
		for (uint32_t index = 0; const auto & id : relComp.Children)
		{
			if (id == entity.GetId())
			{
				auto childEnt = Volt::Entity{ id, myScene };
				if (childEnt.HasComponent<RelationshipComponent>())
				{
					// Do this by hand to not get recursive calls
					childEnt.GetComponent<RelationshipComponent>().Parent = Wire::NullID;
				}

				relComp.Children.erase(relComp.Children.begin() + index);
				break;
			}
			index++;
		}
	}

	const Ref<PhysicsActor> Entity::GetPhysicsActor() const
	{
		return Physics::GetScene()->GetActor(*this);
	}

	void Entity::UpdatePhysicsTranslation(bool updateThis)
	{
		if (updateThis && myScene->IsPlaying() && (HasComponent<RigidbodyComponent>() || HasComponent<CharacterControllerComponent>()))
		{
			auto actor = Physics::GetScene()->GetActor(*this);
			if (actor)
			{
				const glm::vec3 tempPosition = GetPosition();
				actor->SetPosition(tempPosition, true, false);
			}
			else
			{
				auto cc = Physics::GetScene()->GetControllerActor(*this);
				cc->SetFootPosition(GetPosition());
			}
		}
	}

	void Entity::UpdatePhysicsRotation(bool updateThis)
	{
		if (updateThis && myScene->IsPlaying() && HasComponent<RigidbodyComponent>())
		{
			auto actor = Physics::GetScene()->GetActor(*this);
			if (actor)
			{
				const glm::quat tempRotation = GetRotation();
				actor->SetRotation(tempRotation, true, false);
			}
		}
	}

	void Entity::SetPosition(const glm::vec3& position, bool updatePhysics)
	{
		Volt::Entity parent = GetParent();
		Scene::TQS parentTransform{};

		if (parent)
		{
			parentTransform = myScene->GetWorldSpaceTRS(parent);
		}

		// Calculate new local position
		const glm::vec3 translatedPoint = position - parentTransform.position;
		const glm::vec3 invertedScale = 1.f / parentTransform.scale;
		const glm::vec3 rotatedPoint = glm::conjugate(parentTransform.rotation) * translatedPoint;

		const glm::vec3 localPoint = rotatedPoint * invertedScale;
		SetLocalPosition(localPoint, updatePhysics);
	}

	void Entity::SetRotation(const glm::quat& rotation, bool updatePhysics)
	{
		Volt::Entity parent = GetParent();
		Scene::TQS parentTransform{};

		if (parent)
		{
			parentTransform = myScene->GetWorldSpaceTRS(parent);
		}

		const glm::quat localRotation = glm::conjugate(parentTransform.rotation) * rotation;
		SetLocalRotation(localRotation, updatePhysics);
	}

	void Entity::SetScale(const glm::vec3& scale)
	{
		Volt::Entity parent = GetParent();
		Scene::TQS parentTransform{};

		if (parent)
		{
			parentTransform = myScene->GetWorldSpaceTRS(parent);
		}

		const glm::vec3 inverseScale = 1.f / parentTransform.scale;
		const glm::vec3 localScale = scale * inverseScale;

		SetLocalScale(localScale);
	}

	void Entity::SetLocalPosition(const glm::vec3& position, bool updatePhysics)
	{
		GetComponent<TransformComponent>().position = position;
		UpdatePhysicsTranslation(updatePhysics);

		myScene->InvalidateEntityTransform(myId);
	}

	void Entity::SetLocalRotation(const glm::quat& rotation, bool updatePhysics)
	{
		auto& transComp = GetComponent<TransformComponent>();
		transComp.rotation = rotation;
		UpdatePhysicsRotation(updatePhysics);

		myScene->InvalidateEntityTransform(myId);
	}

	void Entity::SetLocalScale(const glm::vec3& scale)
	{
		GetComponent<TransformComponent>().scale = scale;

		myScene->InvalidateEntityTransform(myId);
	}

	const glm::mat4 Entity::GetLocalTransform() const
	{
		if (HasComponent<TransformComponent>())
		{
			const auto& comp = GetComponent<TransformComponent>();
			return comp.GetTransform();
		}

		return glm::mat4{ 1.f };
	}

	const glm::mat4 Entity::GetTransform() const
	{
		return myScene->GetWorldSpaceTransform(*this);
	}

	const glm::vec3 Entity::GetLocalForward() const
	{
		if (HasComponent<TransformComponent>())
		{
			const auto& comp = GetComponent<TransformComponent>();
			return comp.GetForward();
		}

		return glm::vec3{ 0.f, 0.f, 1.f };
	}

	const glm::vec3 Entity::GetLocalRight() const
	{
		if (HasComponent<TransformComponent>())
		{
			const auto& comp = GetComponent<TransformComponent>();
			return comp.GetRight();
		}

		return glm::vec3{ 1.f, 0.f, 0.f };
	}

	const glm::vec3 Entity::GetLocalUp() const
	{
		if (HasComponent<TransformComponent>())
		{
			const auto& comp = GetComponent<TransformComponent>();
			return comp.GetUp();
		}

		return glm::vec3{ 0.f, 1.f, 0.f };
	}

	const glm::vec3 Entity::GetForward() const
	{
		return myScene->GetWorldForward(*this);
	}

	const glm::vec3 Entity::GetRight() const
	{
		return myScene->GetWorldRight(*this);
	}

	const glm::vec3 Entity::GetUp() const
	{
		return myScene->GetWorldUp(*this);
	}

	Volt::Entity& Entity::operator=(const Entity& entity)
	{
		myId = entity.myId;
		myScene = entity.myScene;
		return *this;
	}

	void Entity::Copy(Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, MonoScriptFieldCache& scrScriptFieldCache, MonoScriptFieldCache& targetScriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity, std::vector<WireGUID> aExcludedComponents, bool aShouldReset)
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

			if (guid == VisualScriptingComponent::comp_guid && pool->HasComponent(aSrcEntity))
			{
				if (!aTargetRegistry.HasComponent(guid, aTargetEntity))
				{
					aTargetRegistry.AddComponent(guid, aTargetEntity);
				}

				VisualScriptingComponent* srcComp = (VisualScriptingComponent*)pool->GetComponent(aSrcEntity);
				VisualScriptingComponent* otherComponent = (VisualScriptingComponent*)aTargetRegistry.GetComponentPtr(guid, aTargetEntity);

				if (srcComp->graph)
				{
					otherComponent->graph = CreateRef<GraphKey::Graph>();
					GraphKey::Graph::Copy(srcComp->graph, otherComponent->graph);
				}

				continue;
			}

			if (guid == MonoScriptComponent::comp_guid && pool->HasComponent(aSrcEntity))
			{
				if (!aTargetRegistry.HasComponent(guid, aTargetEntity))
				{
					aTargetRegistry.AddComponent(guid, aTargetEntity);
				}

				MonoScriptComponent* otherComponent = (MonoScriptComponent*)aTargetRegistry.GetComponentPtr(guid, aTargetEntity);
				MonoScriptComponent* scriptComp = (MonoScriptComponent*)pool->GetComponent(aSrcEntity);

				otherComponent->scriptNames = scriptComp->scriptNames;

				for (uint32_t i = 0; i < scriptComp->scriptIds.size(); i++)
				{
					otherComponent->scriptIds.emplace_back(UUID());

					if (Volt::MonoScriptEngine::EntityClassExists(scriptComp->scriptNames[i]))
					{
						const auto& classFields = Volt::MonoScriptEngine::GetScriptClass(scriptComp->scriptNames[i])->GetFields();

						if (!scrScriptFieldCache.GetCache().contains(scriptComp->scriptIds[i]))
						{
							continue;
						}
						
						auto& fromEntityFields = scrScriptFieldCache.GetCache().at(scriptComp->scriptIds[i]);
						auto& toEntityFields = targetScriptFieldCache.GetCache()[otherComponent->scriptIds[i]];

						for (const auto& [name, field] : classFields)
						{
							if (fromEntityFields.contains(name))
							{
								auto& entField = fromEntityFields.at(name);

								toEntityFields[name] = CreateRef<MonoScriptFieldInstance>();
								toEntityFields.at(name)->field.enumName = fromEntityFields.at(name)->field.enumName;

								switch (field.type)
								{
									case MonoFieldType::Bool: toEntityFields[name]->SetValue(*entField->data.As<bool>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::String: toEntityFields[name]->SetValue(*entField->data.As<const char>(), entField->data.GetSize(), field.type); break;

									case MonoFieldType::Int: toEntityFields[name]->SetValue(*entField->data.As<int32_t>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::UInt: toEntityFields[name]->SetValue(*entField->data.As<uint32_t>(), entField->data.GetSize(), field.type); break;

									case MonoFieldType::Short: toEntityFields[name]->SetValue(*entField->data.As<int16_t>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::UShort: toEntityFields[name]->SetValue(*entField->data.As<uint16_t>(), entField->data.GetSize(), field.type); break;

									case MonoFieldType::Char: toEntityFields[name]->SetValue(*entField->data.As<int8_t>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::UChar: toEntityFields[name]->SetValue(*entField->data.As<uint8_t>(), entField->data.GetSize(), field.type); break;

									case MonoFieldType::Float: toEntityFields[name]->SetValue(*entField->data.As<float>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::Double: toEntityFields[name]->SetValue(*entField->data.As<double>(), entField->data.GetSize(), field.type); break;

									case MonoFieldType::Vector2: toEntityFields[name]->SetValue(*entField->data.As<glm::vec2>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::Vector3: toEntityFields[name]->SetValue(*entField->data.As<glm::vec3>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::Vector4: toEntityFields[name]->SetValue(*entField->data.As<glm::vec4>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::Quaternion: toEntityFields[name]->SetValue(*entField->data.As<glm::quat>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::Entity: toEntityFields[name]->SetValue(*entField->data.As<Wire::EntityId>(), entField->data.GetSize(), field.type); break;

									case MonoFieldType::Animation:
									case MonoFieldType::Prefab:
									case MonoFieldType::Scene:
									case MonoFieldType::Mesh:
									case MonoFieldType::Font:
									case MonoFieldType::Material:
									case MonoFieldType::Texture:
									case MonoFieldType::PostProcessingMaterial:
									case MonoFieldType::Video:
									case MonoFieldType::Asset: toEntityFields[name]->SetValue(*entField->data.As<Volt::AssetHandle>(), entField->data.GetSize(), field.type); break;

									case MonoFieldType::Color: toEntityFields[name]->SetValue(*entField->data.As<glm::vec4>(), entField->data.GetSize(), field.type); break;
									case MonoFieldType::Enum: toEntityFields[name]->SetValue(*entField->data.As<uint32_t>(), entField->data.GetSize(), field.type); break;
								}
							}
						}
					}
				}

				continue;
			}

			// Handle collider components
			if (guid == RigidbodyComponent::comp_guid ||
				guid == BoxColliderComponent::comp_guid ||
				guid == SphereColliderComponent::comp_guid ||
				guid == CapsuleColliderComponent::comp_guid)
			{
				CopyPhysicsComponents(guid, aSrcRegistry, aTargetRegistry, aSrcEntity, aTargetEntity);
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
						case Wire::ComponentRegistry::PropertyType::Vector2: (*(glm::vec2*)&otherComponent[prop.offset]) = (*(glm::vec2*)&thisComponent[prop.offset]); break;
						case Wire::ComponentRegistry::PropertyType::Vector3: (*(glm::vec3*)&otherComponent[prop.offset]) = (*(glm::vec3*)&thisComponent[prop.offset]); break;
						case Wire::ComponentRegistry::PropertyType::Vector4: (*(glm::vec4*)&otherComponent[prop.offset]) = (*(glm::vec4*)&thisComponent[prop.offset]); break;
						case Wire::ComponentRegistry::PropertyType::Quaternion: (*(glm::quat*)&otherComponent[prop.offset]) = (*(glm::quat*)&thisComponent[prop.offset]); break;
						case Wire::ComponentRegistry::PropertyType::String: (*(std::string*)&otherComponent[prop.offset]) = (*(std::string*)&thisComponent[prop.offset]); break;
						case Wire::ComponentRegistry::PropertyType::Int64: (*(int64_t*)&otherComponent[prop.offset]) = (*(int64_t*)&thisComponent[prop.offset]); break;
						case Wire::ComponentRegistry::PropertyType::UInt64: (*(uint64_t*)&otherComponent[prop.offset]) = (*(uint64_t*)&thisComponent[prop.offset]); break;
						case Wire::ComponentRegistry::PropertyType::AssetHandle: (*(AssetHandle*)&otherComponent[prop.offset]) = (*(AssetHandle*)&thisComponent[prop.offset]); break;
						case Wire::ComponentRegistry::PropertyType::Color3: (*(glm::vec3*)&otherComponent[prop.offset]) = (*(glm::vec3*)&thisComponent[prop.offset]); break;
						case Wire::ComponentRegistry::PropertyType::Color4: (*(glm::vec4*)&otherComponent[prop.offset]) = (*(glm::vec4*)&thisComponent[prop.offset]); break;
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
								case Wire::ComponentRegistry::PropertyType::Vector2: (*(std::vector<glm::vec2>*) & otherComponent[prop.offset]) = (*(std::vector<glm::vec2>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::Vector3: (*(std::vector<glm::vec3>*) & otherComponent[prop.offset]) = (*(std::vector<glm::vec3>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::Vector4: (*(std::vector<glm::vec4>*) & otherComponent[prop.offset]) = (*(std::vector<glm::vec4>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::Quaternion: (*(std::vector<glm::quat>*) & otherComponent[prop.offset]) = (*(std::vector<glm::quat>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::String: (*(std::vector<std::string>*) & otherComponent[prop.offset]) = (*(std::vector<std::string>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::Int64: (*(std::vector<int64_t>*) & otherComponent[prop.offset]) = (*(std::vector<int64_t>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::UInt64: (*(std::vector<uint64_t>*) & otherComponent[prop.offset]) = (*(std::vector<uint64_t>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::AssetHandle: (*(std::vector<AssetHandle>*) & otherComponent[prop.offset]) = (*(std::vector<AssetHandle>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::Color4: (*(std::vector<glm::vec2>*) & otherComponent[prop.offset]) = (*(std::vector<glm::vec2>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::Color3: (*(std::vector<glm::vec3>*) & otherComponent[prop.offset]) = (*(std::vector<glm::vec3>*) & thisComponent[prop.offset]); break;
								case Wire::ComponentRegistry::PropertyType::Folder: (*(std::vector<glm::vec4>*) & otherComponent[prop.offset]) = (*(std::vector<glm::vec4>*) & thisComponent[prop.offset]); break;
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

	Wire::EntityId Entity::Duplicate(Wire::Registry& aRegistry, MonoScriptFieldCache& scriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity, std::vector<WireGUID> aExcludedComponents, bool shouldReset)
	{
		Wire::EntityId parent = Wire::NullID;

		if (aRegistry.HasComponent<RelationshipComponent>(aSrcEntity))
		{
			auto& relComp = aRegistry.GetComponent<RelationshipComponent>(aSrcEntity);
			parent = relComp.Parent;
		}

		Wire::EntityId newEnt = DuplicateInternal(aRegistry, scriptFieldCache, aSrcEntity, parent, aTargetEntity);

		if (parent != Wire::NullID)
		{
			auto& relComp = aRegistry.GetComponent<RelationshipComponent>(parent);
			relComp.Children.emplace_back(newEnt);
		}

		return newEnt;
	}

	Wire::EntityId Entity::Duplicate(Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, MonoScriptFieldCache& scrScriptFieldCache, MonoScriptFieldCache& targetScriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity, std::vector<WireGUID> aExcludedComponents, bool shouldReset)
	{
		Wire::EntityId newEnt = DuplicateInternal(aSrcRegistry, aTargetRegistry, scrScriptFieldCache, targetScriptFieldCache, aSrcEntity, Wire::NullID, aTargetEntity);
		return newEnt;
	}

	void Entity::CopyPhysicsComponents(const WireGUID& guid, Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity)
	{
		if (guid == RigidbodyComponent::comp_guid)
		{
			if (!aSrcRegistry.HasComponent<RigidbodyComponent>(aSrcEntity))
			{
				return;
			}

			const auto& srcComp = aSrcRegistry.GetComponent<RigidbodyComponent>(aSrcEntity);

			if (!aTargetRegistry.HasComponent<RigidbodyComponent>(aTargetEntity))
			{
				aTargetRegistry.AddComponent<RigidbodyComponent>(aTargetEntity, srcComp.bodyType, srcComp.layerId, srcComp.mass, srcComp.linearDrag, srcComp.lockFlags, srcComp.angularDrag, srcComp.disableGravity, srcComp.isKinematic, srcComp.collisionType);
			}
			else
			{
				auto& targetComp = aTargetRegistry.GetComponent<RigidbodyComponent>(aTargetEntity);
				targetComp.bodyType = srcComp.bodyType;
				targetComp.layerId = srcComp.layerId;
				targetComp.mass = srcComp.mass;
				targetComp.linearDrag = srcComp.linearDrag;
				targetComp.lockFlags = srcComp.lockFlags;
				targetComp.angularDrag = srcComp.angularDrag;
				targetComp.disableGravity = srcComp.disableGravity;
				targetComp.isKinematic = srcComp.isKinematic;
			}
		}
		else if (guid == BoxColliderComponent::comp_guid)
		{
			if (!aSrcRegistry.HasComponent<BoxColliderComponent>(aSrcEntity))
			{
				return;
			}

			const auto& srcComp = aSrcRegistry.GetComponent<BoxColliderComponent>(aSrcEntity);

			if (!aTargetRegistry.HasComponent<BoxColliderComponent>(aTargetEntity))
			{
				aTargetRegistry.AddComponent<BoxColliderComponent>(aTargetEntity, srcComp.halfSize, srcComp.offset, srcComp.isTrigger, srcComp.material);
			}
			else
			{
				auto& targetComp = aTargetRegistry.GetComponent<BoxColliderComponent>(aTargetEntity);
				targetComp.halfSize = srcComp.halfSize;
				targetComp.offset = srcComp.offset;
				targetComp.isTrigger = srcComp.isTrigger;
				targetComp.material = srcComp.material;
			}
		}
		else if (guid == SphereColliderComponent::comp_guid)
		{
			if (!aSrcRegistry.HasComponent<SphereColliderComponent>(aSrcEntity))
			{
				return;
			}

			const auto& srcComp = aSrcRegistry.GetComponent<SphereColliderComponent>(aSrcEntity);
			if (!aTargetRegistry.HasComponent<SphereColliderComponent>(aTargetEntity))
			{
				aTargetRegistry.AddComponent<SphereColliderComponent>(aTargetEntity, srcComp.radius, srcComp.offset, srcComp.isTrigger, srcComp.material);
			}
			else
			{
				auto& targetComp = aTargetRegistry.GetComponent<SphereColliderComponent>(aTargetEntity);
				targetComp.radius = srcComp.radius;
				targetComp.offset = srcComp.offset;
				targetComp.isTrigger = srcComp.isTrigger;
				targetComp.material = srcComp.material;
			}
		}
		else if (guid == CapsuleColliderComponent::comp_guid)
		{
			if (!aSrcRegistry.HasComponent<CapsuleColliderComponent>(aSrcEntity))
			{
				return;
			}

			const auto& srcComp = aSrcRegistry.GetComponent<CapsuleColliderComponent>(aSrcEntity);
			if (!aTargetRegistry.HasComponent<CapsuleColliderComponent>(aTargetEntity))
			{
				aTargetRegistry.AddComponent<CapsuleColliderComponent>(aTargetEntity, srcComp.radius, srcComp.height, srcComp.offset, srcComp.isTrigger, srcComp.material);
			}
			else
			{
				auto& targetComp = aTargetRegistry.GetComponent<CapsuleColliderComponent>(aTargetEntity);
				targetComp.radius = srcComp.radius;
				targetComp.height = srcComp.height;
				targetComp.offset = srcComp.offset;
				targetComp.isTrigger = srcComp.isTrigger;
				targetComp.material = srcComp.material;
			}
		}
		else if (guid == MeshColliderComponent::comp_guid)
		{
			if (!aSrcRegistry.HasComponent<MeshColliderComponent>(aSrcEntity))
			{
				return;
			}

			const auto& srcComp = aSrcRegistry.GetComponent<MeshColliderComponent>(aSrcEntity);
			if (!aTargetRegistry.HasComponent<MeshColliderComponent>(aTargetEntity))
			{
				aTargetRegistry.AddComponent<MeshColliderComponent>(aTargetEntity, srcComp.colliderMesh, srcComp.isConvex, srcComp.isTrigger, srcComp.material, srcComp.subMeshIndex);
			}
			else
			{
				auto& targetComp = aTargetRegistry.GetComponent<MeshColliderComponent>(aTargetEntity);
				targetComp.colliderMesh = srcComp.colliderMesh;
				targetComp.isConvex = srcComp.isConvex;
				targetComp.isTrigger = srcComp.isTrigger;
				targetComp.material = srcComp.material;
				targetComp.subMeshIndex = srcComp.subMeshIndex;
			}
		}
	}

	Wire::EntityId Entity::DuplicateInternal(Wire::Registry& aRegistry, MonoScriptFieldCache& scriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aParent, Wire::EntityId aTargetEntity, std::vector<WireGUID> aExcludedComponents, bool shouldReset)
	{
		Wire::EntityId newEnt = (aTargetEntity != Wire::NullID) ? aRegistry.AddEntity(aTargetEntity) : aRegistry.CreateEntity();
		aRegistry.AddComponent<RelationshipComponent>(newEnt);

		if (aExcludedComponents.empty())
		{
			aExcludedComponents.emplace_back(RelationshipComponent::comp_guid);
		}

		Copy(aRegistry, aRegistry, scriptFieldCache, scriptFieldCache, aSrcEntity, newEnt, aExcludedComponents, shouldReset);

		if (aRegistry.HasComponent<NetActorComponent>(newEnt))
		{
			aRegistry.GetComponent<NetActorComponent>(newEnt).repId = Nexus::RandRepID();
		}

		if (aRegistry.HasComponent<RelationshipComponent>(aSrcEntity))
		{
			auto& relComp = aRegistry.GetComponent<RelationshipComponent>(aSrcEntity);
			std::vector<Wire::EntityId> newChildren;

			for (const auto& child : relComp.Children)
			{
				newChildren.emplace_back(DuplicateInternal(aRegistry, scriptFieldCache, child, newEnt, Wire::NullID, aExcludedComponents, shouldReset));
			}

			auto& newRelComp = aRegistry.GetComponent<RelationshipComponent>(newEnt);
			newRelComp.Children = newChildren;
			newRelComp.Parent = aParent;
		}

		return newEnt;
	}

	Wire::EntityId Entity::DuplicateInternal(Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, MonoScriptFieldCache& scrScriptFieldCache, MonoScriptFieldCache& targetScriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aParent, Wire::EntityId aTargetEntity, std::vector<WireGUID> aExcludedComponents, bool shouldReset)
	{
		Wire::EntityId newEnt = (aTargetEntity != Wire::NullID) ? aTargetRegistry.AddEntity(aTargetEntity) : aTargetRegistry.CreateEntity();
		aTargetRegistry.AddComponent<RelationshipComponent>(newEnt);

		if (aExcludedComponents.empty())
		{
			aExcludedComponents.emplace_back(RelationshipComponent::comp_guid);
		}

		Copy(aSrcRegistry, aTargetRegistry, scrScriptFieldCache, targetScriptFieldCache, aSrcEntity, newEnt, aExcludedComponents, shouldReset);

		if (aSrcRegistry.HasComponent<RelationshipComponent>(aSrcEntity))
		{
			auto& relComp = aSrcRegistry.GetComponent<RelationshipComponent>(aSrcEntity);
			std::vector<Wire::EntityId> newChildren;

			for (const auto& child : relComp.Children)
			{
				newChildren.emplace_back(DuplicateInternal(aSrcRegistry, aTargetRegistry, scrScriptFieldCache, targetScriptFieldCache, child, newEnt, Wire::NullID, aExcludedComponents, shouldReset));
			}

			auto& newRelComp = aTargetRegistry.GetComponent<RelationshipComponent>(newEnt);
			newRelComp.Children = newChildren;
			newRelComp.Parent = aParent;
		}

		return newEnt;
	}
}
