#include "vtpch.h"
#include "NetSerialization.h"

#include "Volt/Net/Replicated/RepEntity.h"
#include "Volt/Net/NetHandler.h"
#include "Volt/Core/Application.h"

#include <AssetSystem/AssetManager.h>
#include "Volt/Asset/Prefab.h"

#include "Volt/Net/SceneInteraction/NetActorComponent.h"

#include "Volt/Net/SceneInteraction/NetContract.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Components/PhysicsComponents.h"
#include "Volt/Scene/Reflection/ComponentRegistry.h"
#include "Volt/Physics/PhysicsScene.h"
#include "Volt/Physics/Physics.h"


#include <stdint.h>
template<typename T>
Ref<T> GetGenericRepData(Nexus::Packet& in_packet, Volt::eNetSerializerDescriptor in_type)
{
	static_assert(std::derived_from<T, Volt::RepData> && "Bad type");

	T data;
	in_packet > data;
	data.dataType = in_type;
	return CreateRef<T>(data);
}

Ref<Volt::RepData> DeserializePacketBody(Nexus::Packet& in_packet)
{
	if (in_packet.body.size() <= 0) return nullptr;

	Volt::eNetSerializerDescriptor description;
	in_packet >> description;
	switch (description)
	{
		//case Volt::eNetSerializerDescriptor:
		case Volt::eNetSerializerDescriptor::RPC:		return GetGenericRepData<Volt::RepRPCData>(in_packet, description);
		case Volt::eNetSerializerDescriptor::TRANSFORM: return GetGenericRepData<Volt::RepTransformComponentData>(in_packet, description);

		case Volt::eNetSerializerDescriptor::PREFAB:
		{
			auto data = GetGenericRepData<Volt::RepPrefabData>(in_packet, description);
			while (!in_packet.body.empty()) data->componentData.push_back(DeserializePacketBody(in_packet));
			return data;
		} break;
		case Volt::eNetSerializerDescriptor::VARIABLE:
		{
			Volt::RepVariableData variableData;
			in_packet > variableData;
			variableData.dataType = Volt::eNetSerializerDescriptor::VARIABLE;
			return CreateRef<Volt::RepVariableData>(variableData);
		} break;
		/*
		case Volt::eNetSerializerDescriptor:
		{

		} break;
		*/
		default: break;
	}

	return nullptr;
}

bool ApplyComponentData(Ref<Volt::RepData> in_data, Nexus::ReplicationRegisty& in_registry, Volt::eNetErrorCode* out_error)
{
	if (out_error == nullptr) out_error = Volt::Application::Get().GetNetHandler().GetErrorPtr();
	if (!in_data) return false;
	switch (in_data->dataType)
	{
		case Volt::eNetSerializerDescriptor::REQUEST:
		{
			auto requestData = reinterpret_pointer_cast<Volt::RepRequestData>(in_data);

		}break;
		case Volt::eNetSerializerDescriptor::VARIABLE:
		{
			if (!Volt::MonoScriptEngine::IsRunning()) return false;
			auto variableData = reinterpret_pointer_cast<Volt::RepVariableData>(in_data);
			auto repVar = in_registry.GetAs<Volt::RepVariable>(in_data->repId);
			if (!repVar) { *out_error = Volt::eNetErrorCode::MISSING_NET_VARIABLE; return false; }
			if (!repVar->SetValue(variableData->data->data)) { *out_error = Volt::eNetErrorCode::MISSING_NET_VARIABLE; return false; }
		}break;
		case Volt::eNetSerializerDescriptor::TRANSFORM:
		{
			auto transformData = reinterpret_pointer_cast<Volt::RepTransformComponentData>(in_data);
			auto netEnt = reinterpret_pointer_cast<Volt::RepEntity>(in_registry.Get(in_data->repId));
			if (!netEnt) { *out_error = Volt::eNetErrorCode::MISSING_NET_ENTITY; return false; }

			auto sceneEnt = Volt::SceneManager::GetActiveScene()->GetEntityFromUUID(netEnt->GetEntityId());
			if (!sceneEnt.IsValid()) { *out_error = Volt::eNetErrorCode::MISSING_SCENE_ENTITY; return false; }

			if (sceneEnt.HasComponent<Volt::TransformComponent>())
			{
				/*if (sceneEnt.HasComponent<Volt::CharacterControllerComponent>())
				{
					auto pscasnjkasdnk = Volt::Physics::GetScene();
					Ref<Volt::PhysicsControllerActor> controllerActor = nullptr;
					if (pscasnjkasdnk) controllerActor = pscasnjkasdnk->GetControllerActor(sceneEnt);
					if (controllerActor && controllerActor->IsValid())
					{
						controllerActor->SetFootPosition(transformData->transform.position);
					}
					else
					{
						sceneEnt.SetPosition(transformData->transform.position);
					}
				}
				else*/
				//{
				//}
				if (transformData->setPos != 0) sceneEnt.SetPosition(transformData->transform.position);
				if (transformData->setRot != 0) sceneEnt.SetRotation(transformData->transform.rotation);
				if (transformData->setScale != 0) sceneEnt.SetScale(transformData->transform.scale);
				return true;
			}
			*out_error = Volt::eNetErrorCode::MISSING_COMPONENT;
			return false;
		}break;
		/*case Volt::eNetSerializerDescriptor:
		{

		}break;*/
		default:
			VT_LOG(Error, "not valid RepData");
			return false;
			break;
	}
	return true;
}

Nexus::Packet ConstructErrorDescription(Ref<Volt::RepData>, Volt::eNetErrorCode*)
{
	return Nexus::Packet();
}


VT_OPTIMIZE_OFF
#pragma region Variable
Nexus::Packet SerializeVariablePacket(Volt::RepVariable& variable, Nexus::TYPE::REP_ID repId)
{
	auto& fieldType = variable.GetField().type;
	Volt::RepVariableData varData(10, fieldType);

	if (fieldType.typeIndex == typeid(bool))
	{
		bool data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(int8_t))
	{
		int8_t data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(uint8_t))
	{
		uint8_t data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(int16_t))
	{
		int16_t data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(int32_t))
	{
		int32_t data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(uint32_t))
	{
		uint32_t data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(int64_t))
	{
		uint64_t data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(uint64_t))
	{
		uint64_t data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(float))
	{
		float data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(double))
	{
		double data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}
	else if (fieldType.typeIndex == typeid(glm::quat))
	{
		glm::quat data;
		variable.GetValue(data);
		varData = Volt::RepVariableData(data, fieldType);
	}

	varData.repId = repId;
	varData.fieldType = variable.GetField().type;

	Nexus::Packet packet;
	packet.id = Nexus::ePacketID::UPDATE;
	packet < varData;
	return packet;
}

Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::RepVariableData& data)
{
	packet.Append(data.data->data, data.data->size);
	packet << data.fieldType.typeName << data.repId << Volt::eNetSerializerDescriptor::VARIABLE;
	return packet;
}

Nexus::Packet& operator>(Nexus::Packet& packet, Volt::RepVariableData& varData)
{
	std::string fieldTypeName;

	packet >> varData.repId >> fieldTypeName;

	varData.fieldType = Volt::MonoTypeRegistry::GetTypeInfo(fieldTypeName);
	auto& fieldType = varData.fieldType;

	if (fieldType.typeIndex == typeid(bool))
	{
		bool data;
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}
	else if (fieldType.typeIndex == typeid(int8_t))
	{
		int8_t data;
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}
	else if (fieldType.typeIndex == typeid(uint8_t))
	{
		uint8_t data;
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}
	else if (fieldType.typeIndex == typeid(int16_t))
	{
		int16_t data;
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}
	else if (fieldType.typeIndex == typeid(int32_t))
	{
		int32_t data;
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}
	else if (fieldType.typeIndex == typeid(uint32_t))
	{
		uint32_t data;
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}
	else if (fieldType.typeIndex == typeid(int64_t))
	{
		int64_t data;
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}
	else if (fieldType.typeIndex == typeid(uint64_t))
	{
		uint64_t data;
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}
	else if (fieldType.typeIndex == typeid(float))
	{
		float data;
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}
	else if (fieldType.typeIndex == typeid(glm::quat))
	{
		glm::quat data = { 0,0,0,0 };
		packet >> data;
		varData.data = CreateRef<Volt::VariableData>(data);
	}

	return packet;
}

Volt::RepVariableData::RepVariableData(const RepVariableData& in_data)
{
	data = in_data.data;
	dataType = in_data.dataType;
	fieldType = in_data.fieldType;
	repId = in_data.repId;
}

#pragma endregion

#pragma region Transform
Nexus::Packet SerializeTransformPacket(Volt::EntityID in_entityId, Nexus::TYPE::REP_ID in_repId, int pos, int rot, int scale)
{
	Volt::Entity entity = Volt::SceneManager::GetActiveScene()->GetEntityFromUUID(in_entityId);
	Volt::RepTransformComponentData entTransform;
	entTransform.repId = in_repId;
	entTransform.setPos = pos;
	entTransform.setRot = rot;
	entTransform.setScale = scale;

	Volt::TransformComponent comp = entity.GetComponent<Volt::TransformComponent>();
	/*
	comp.position = entity.GetPosition();
	comp.rotation = entity.GetRotation();
	comp.scale = entity.GetScale();
	*/
	entTransform.transform = comp;

	Nexus::Packet transformPacket;
	transformPacket.id = Nexus::ePacketID::MOVE;
	transformPacket < entTransform;
	return transformPacket;
}

Volt::RepTransformComponentData CreateTransformComponentData(Nexus::TYPE::REP_ID in_repId, const Volt::TransformComponent& component)
{
	Volt::RepTransformComponentData data;
	data.repId = in_repId;

	data.transform = component;
	return data;
}

Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::RepTransformComponentData& transform)
{
	packet << transform.repId;
	packet << transform.setPos << transform.setRot << transform.setScale;

	packet << transform.transform.position.x;
	packet << transform.transform.position.y;
	packet << transform.transform.position.z;

	packet << transform.transform.rotation.x;
	packet << transform.transform.rotation.y;
	packet << transform.transform.rotation.z;
	packet << transform.transform.rotation.w;

	packet << transform.transform.scale.x;
	packet << transform.transform.scale.y;
	packet << transform.transform.scale.z;

	packet << Volt::eNetSerializerDescriptor::TRANSFORM;

	return packet;
}

Nexus::Packet& operator>(Nexus::Packet& packet, Volt::RepTransformComponentData& transform)
{

	packet >> transform.transform.scale.z;
	packet >> transform.transform.scale.y;
	packet >> transform.transform.scale.x;

	packet >> transform.transform.rotation.w;
	packet >> transform.transform.rotation.z;
	packet >> transform.transform.rotation.y;
	packet >> transform.transform.rotation.x;

	packet >> transform.transform.position.z;
	packet >> transform.transform.position.y;
	packet >> transform.transform.position.x;

	packet >> transform.setScale >> transform.setRot >> transform.setPos;
	packet >> transform.repId;

	return packet;
}

#pragma endregion 

#pragma region RPC
Nexus::Packet SerializeRPCPacket(std::string in_project, std::string in_class, std::string in_method, Nexus::TYPE::REP_ID in_entity, Nexus::TYPE::RPC_ID in_id)
{
	Nexus::Packet rpcPacket;
	rpcPacket < CreateRPCData(in_project, in_class, in_method, in_entity, in_id);
	return rpcPacket;
}

Volt::RepRPCData CreateRPCData(std::string in_project, std::string in_class, std::string in_method, Nexus::TYPE::REP_ID in_entity, Nexus::TYPE::RPC_ID in_id)
{
	Volt::RepRPCData rpcData;
	rpcData.monoProject = in_project;
	rpcData.monoClass = in_class;
	rpcData.monoMethod = in_method;
	rpcData.rpcId = in_id;
	rpcData.repId = in_entity;
	return rpcData;
}

Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::RepRPCData& rpcData)
{
	uint8_t pSize, cSize, mSize;
	pSize = (uint8_t)rpcData.monoProject.size();
	cSize = (uint8_t)rpcData.monoClass.size();
	mSize = (uint8_t)rpcData.monoMethod.size();

	packet << rpcData.monoProject << pSize << rpcData.monoClass << cSize << rpcData.monoMethod << mSize;

	packet << rpcData.rpcId;
	packet << rpcData.repId;
	packet << Volt::eNetSerializerDescriptor::RPC;
	return packet;
}

Nexus::Packet& operator>(Nexus::Packet& packet, Volt::RepRPCData& rpcData)
{
	Vector<std::string> monoElements;
	packet >> rpcData.repId >> rpcData.rpcId;

	std::string monoData;
	uint8_t length;
	for (int i = 0; i < 3; i++)
	{
		packet >> length;
		monoElements.push_back(packet.GetString(length));
	}

	if (monoElements.size() != 3)
	{
		LogError("Bad RPC call");
		rpcData.monoProject = "";
		rpcData.monoClass = "";
		rpcData.monoMethod = "";
		return packet;
	}
	rpcData.monoProject = monoElements[2];
	rpcData.monoClass = monoElements[1];
	rpcData.monoMethod = monoElements[0];
	return packet;
}
#pragma endregion 

void HandleScript(Volt::EntityID in_id, const std::string& id, bool in_keep)
{
	if (in_keep) return;
	auto ent = Volt::SceneManager::GetActiveScene()->GetEntityFromUUID(in_id);
	auto& scriptComp = ent.GetComponent<Volt::MonoScriptComponent>();

	for (uint32_t i = 0; i < scriptComp.scriptIds.size(); ++i)
	{
		if (scriptComp.scriptNames[i] == id)
		{
			scriptComp.scriptIds.erase(scriptComp.scriptIds.begin() + i);
			scriptComp.scriptNames.erase(scriptComp.scriptNames.begin() + i);
			break;
		}
	}
}

#pragma region  Prefab
void HandleComponent(Volt::EntityID in_id, const std::string& in_comp, bool in_keep)
{
	if (in_keep) return;

	auto ent = Volt::SceneManager::GetActiveScene()->GetEntityFromUUID(in_id);
	ent.RemoveComponent(Volt::ComponentRegistry::GetGUIDFromTypeName(in_comp));
}

void RecursiveOwnerShipControll(Volt::EntityID in_id, const Volt::RepPrefabData& data)
{
	auto contract = Volt::NetContractContainer::GetContract(data.handle);

	auto scenePtr = Volt::SceneManager::GetActiveScene();

	auto ent = scenePtr->GetEntityFromUUID(in_id);
	const auto prefabData = Volt::AssetManager::GetAsset<Volt::Prefab>(data.handle);
	const auto& prefabComponent = ent.GetComponent<Volt::PrefabComponent>();

	auto& registry = scenePtr->GetRegistry();

	auto isOwner = Volt::Application::Get().GetNetHandler().IsOwner(data.repId);
	auto isHost = Volt::Application::Get().GetNetHandler().IsHost();

	// Components
	if (contract)
	{
		if (contract->rules.contains(prefabComponent.prefabEntity))
		{
			for (auto&& curr : registry.storage())
			{
				auto& storage = curr.second;

				if (!storage.contains(ent))
				{
					continue;
				}

				const std::string componentName = std::string(storage.type().name());
				const Volt::ICommonTypeDesc* compTypeDesc = Volt::ComponentRegistry::GetTypeDescFromName(componentName);

				// #TODO_Ivar: Name matching might not work anymore, should move to GUIDs
				if (!Volt::NetContractContainer::RuleExists(data.handle, componentName, prefabComponent.prefabEntity) && compTypeDesc->GetGUID() != Volt::GetTypeGUID<Volt::MonoScriptComponent>())
				{
					continue;
				}

				if (compTypeDesc->GetGUID() == Volt::GetTypeGUID<Volt::MonoScriptComponent>())
				{
					auto& monoScriptComponent = ent.GetComponent<Volt::MonoScriptComponent>();
					for (uint32_t i = 0; i < monoScriptComponent.scriptIds.size(); i++)
					{
						if (!contract->rules.at(prefabComponent.prefabEntity).contains(monoScriptComponent.scriptNames[i]))
						{
							continue;
						}

						HandleScript(in_id, monoScriptComponent.scriptNames[i], contract->rules.at(prefabComponent.prefabEntity).at(monoScriptComponent.scriptNames[i]).ShouldKeep(isHost, isOwner));
					}
				}
				else
				{
					HandleComponent(in_id, componentName, contract->rules.at(prefabComponent.prefabEntity)[componentName].ShouldKeep(isHost, isOwner));
				}
			}
		}
	}

	// Children
	if (!ent.HasComponent<Volt::RelationshipComponent>()) return;
	auto children = ent.GetComponent<Volt::RelationshipComponent>().children;
	for (const auto& c : children)
	{
		RecursiveOwnerShipControll(c, data);
	}
}

// #mmax: should be called on play :P
void RecursiveHandleMono(Volt::EntityID entId, Nexus::TYPE::REP_ID owner, Nexus::TYPE::REP_ID& varId, Nexus::ReplicationRegisty* registry, bool manageInstances)
{
	Volt::Entity entity = Volt::SceneManager::GetActiveScene()->GetEntityFromUUID(entId);

	auto repEnt = registry->GetAs<Volt::RepEntity>(owner);

	Volt::MonoScriptEngine::GetOrCreateMonoEntity(entId);
	if (entity.HasComponent<Volt::MonoScriptComponent>())
	{
		auto& comp = entity.GetComponent<Volt::MonoScriptComponent>();
		for (uint32_t i = 0; const auto & sid : comp.scriptIds)
		{
			if (manageInstances)
			{
				Volt::MonoScriptEngine::OnAwakeInstance(sid, entId, comp.scriptNames[i]);
				Volt::MonoScriptEngine::QueueOnCreate(sid);
			}
			//auto& scriptFieldMap = Volt::MonoScriptEngine::GetScriptFieldMap(comp.scriptIds[i]);
			auto sinstance = Volt::MonoScriptEngine::GetInstanceFromId(comp.scriptIds[i]);

			if (registry) for (auto& [name, field] : sinstance->GetClass()->GetFields())
			{
				if (!sinstance) continue;
				if (field.netData.replicatedCondition == Volt::eRepCondition::OFF) continue;
				auto repVariable = Volt::RepVariable(name, owner, repEnt->GetOwner(), sinstance, field, repEnt->GetEntityId());
				varId++;
				registry->Register(varId, repVariable);
				registry->Link(owner, { name, varId });
			}
			i++;
		}
	}

	if (!entity.HasComponent<Volt::RelationshipComponent>()) return;
	auto children = entity.GetComponent<Volt::RelationshipComponent>().children;
	for (auto child : children)
	{
		RecursiveHandleMono(child, owner, varId, registry);
	}
}

bool ConstructPrefab(const Volt::RepPrefabData& data, Nexus::ReplicationRegisty& registry)
{
	auto scenePtr = Volt::SceneManager::GetActiveScene();

	if (auto prefab = Volt::AssetManager::GetAsset<Volt::Prefab>(data.handle))
	{
		auto entity = prefab->Instantiate(scenePtr);

		std::string tagId = " :error";
		if (entity.HasComponent<Volt::NetActorComponent>())
		{
			auto& netActorComp = entity.GetComponent<Volt::NetActorComponent>();
			netActorComp.repId = data.repId;
			netActorComp.clientId = data.ownerId;
			switch (netActorComp.condition)
			{
				case Volt::eRepCondition::CONTINUOUS:	tagId = " :c";	break;
				case Volt::eRepCondition::NOTIFY:		tagId = " :n";	break;
				case Volt::eRepCondition::OFF:			tagId = " :o";	break;
				default:												break;
			}
		}

		registry.Register(data.repId, Volt::RepEntity(entity.GetID(), data.ownerId, data.handle));
		entity.GetComponent<Volt::TagComponent>().tag = entity.GetTag() + tagId + std::to_string(data.repId);

		RecursiveOwnerShipControll(entity.GetID(), data);
		if (Volt::MonoScriptEngine::IsRunning())
		{
			auto varId = data.repId;
			RecursiveHandleMono(entity.GetID(), data.repId, varId, &registry);
		}
		bool status = true;
		for (auto compData : data.componentData)
		{
			compData->repId = data.repId;
			if (ApplyComponentData(compData, registry)) continue;
			status = false;
		}

		if (entity.HasComponent<Volt::RigidbodyComponent>())
		{
			if (!Volt::Application::Get().GetNetHandler().IsOwner(data.repId))
			{
				Volt::Physics::GetScene()->GetActor(entity)->SetGravityDisabled(true);
			}
		}

		if (entity.HasComponent<Volt::CharacterControllerComponent>())
		{
			if (!Volt::Application::Get().GetNetHandler().IsOwner(data.repId))
			{
				Volt::Physics::GetScene()->GetControllerActor(entity)->SetGravity(false);
			}
		}

		return status;
	}

	return false;
}

Volt::RepPrefabData CreatePrefabData(Nexus::TYPE::REP_ID in_repId, Nexus::TYPE::CLIENT_ID in_ownerId, Volt::AssetHandle in_handle)
{
	Volt::RepPrefabData data;
	data.repId = in_repId;
	data.ownerId = in_ownerId;
	data.handle = in_handle;
	return data;
}

Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::RepPrefabData& data)
{
	packet << data.repId << data.ownerId << data.handle << Volt::eNetSerializerDescriptor::PREFAB;
	return packet;
}

Nexus::Packet& operator>(Nexus::Packet& packet, Volt::RepPrefabData& data)
{
	packet >> data.handle >> data.ownerId >> data.repId;
	return packet;
}
#pragma endregion

#pragma region NetEvent
Nexus::Packet SerializeNetEventPacket(Volt::NetEvent in_event)
{
	Nexus::Packet packet;
	packet.id = Nexus::ePacketID::EVENT;
	packet < in_event;
	return packet;
}
Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::NetEvent& netEvent)
{
	packet << netEvent.m_id;
	packet.Append(netEvent.m_data.data(), netEvent.m_data.size());
	packet << (uint8_t)netEvent.m_data.size() << netEvent.m_event;
	return packet;
}
Nexus::Packet& operator>(Nexus::Packet& packet, Volt::NetEvent& netEvent)
{
	packet >> netEvent.m_event;
	uint8_t size;
	packet >> size;
	netEvent.m_data.resize(size);
	memcpy_s(netEvent.m_data.data(), netEvent.m_data.size(), packet.body.data() + packet.body.size() - size, size);
	packet.body.resize(packet.body.size() - size);
	packet >> netEvent.m_id;
	return packet;
}
#pragma endregion


#pragma region 

#pragma endregion

