#pragma once

#include <Nexus/Core/Packet/Packet.hpp>
#include <Nexus/Utility/Types.h>
#include "Nexus/Interface/Replication/ReplicationRegistry.h"
//#include "Volt/Net/SceneInteraction/NetPrefabInstantiation.h"

#include "Volt/Components/CoreComponents.h"
#include "Volt/Utility/StringUtility.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Scene/SceneManager.h"

#include "Volt/Scripting/Mono/MonoScriptClass.h"

#include "Volt/Net/Replicated/RepVariable.h"
#include "Volt/Net/Event/NetEvent.h"

#pragma region Serialization Structures
namespace Volt
{
	enum class eNetSerializerDescriptor : uint8_t
	{
		TRANSFORM,
		RPC,
		VARIABLE,
		PREFAB,
		REQUEST
	};

	enum class eNetErrorCode : uint8_t
	{
		NIL = 0,
		MISSING_SCENE_ENTITY,
		MISSING_NET_ENTITY,
		MISSING_NET_VARIABLE,
		MISSING_COMPONENT,
	};

	struct RepData
	{
		Nexus::TYPE::REP_ID repId = 0;
		eNetSerializerDescriptor dataType = eNetSerializerDescriptor::TRANSFORM;
	};

	struct RepTransformComponentData : public RepData
	{
		TransformComponent transform;
		int setPos = 1;
		int setRot = 1;
		int setScale = 1;

	};

	struct RepRPCData : public RepData
	{
		Nexus::TYPE::RPC_ID rpcId = 0;
		std::string monoProject = "";
		std::string monoClass = "";
		std::string monoMethod = "";
	};

	struct RepPrefabData : public Volt::RepData
	{
		Nexus::TYPE::CLIENT_ID ownerId;
		Nexus::TYPE::REP_ID variableStartId;
		Volt::AssetHandle handle;
		std::vector<Ref<Volt::RepData>> componentData;
	};

	struct VariableData
	{
		template<typename T>
		VariableData(T in_data)
		{
			data = (uint8_t*)malloc(sizeof(in_data));
			size = sizeof(in_data);
			memcpy_s(data, size, &in_data, size);
		}
		~VariableData()
		{
			free(data);
		}

		template<typename T>
		T As()
		{
			return *(T*)data;
		}

		uint8_t* data = nullptr;
		size_t size = 0;
	};

	struct RepVariableData : public RepData
	{
		template<typename T>
		RepVariableData(const T& data, const MonoFieldType& fieldType);
		RepVariableData(const RepVariableData& data);
		//RepVariableData(const MonoFieldType fieldType);
		RepVariableData() {}

		MonoFieldType fieldType{};
		Ref<VariableData> data;
	};

	template<typename T>
	inline RepVariableData::RepVariableData(const T& in_data, const MonoFieldType& in_fieldType)
	{
		data = CreateRef<VariableData>(in_data);
		fieldType = in_fieldType;
	}

	enum class eRequestType
	{
		NIL,
		PLAYER,

	};

	struct RepRequestData : RepData
	{
		eRequestType reqType;
	};
}
#pragma endregion 

Nexus::Packet ConstructErrorDescription(Ref<Volt::RepData> data, Volt::eNetErrorCode* code = nullptr);

#pragma region RepData
Ref<Volt::RepData> DeserializePacketBody(Nexus::Packet& in_packet);
bool ApplyComponentData(Ref<Volt::RepData> in_data, Nexus::ReplicationRegisty& in_registry, Volt::eNetErrorCode* out_error = nullptr);
#pragma endregion

#pragma region Transform
Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::RepTransformComponentData& transform);
Nexus::Packet& operator>(Nexus::Packet& packet, Volt::RepTransformComponentData& transform);
Nexus::Packet SerializeTransformPacket(entt::entity in_entityId, Nexus::TYPE::REP_ID in_repId, int  pos = 1, int  rot = 1, int  scale = 1);
Volt::RepTransformComponentData CreateTransformComponentData(Nexus::TYPE::REP_ID in_repId, const Volt::TransformComponent& component);
#pragma endregion

#pragma region RPC
Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::RepRPCData& rpcData);
Nexus::Packet& operator>(Nexus::Packet& packet, Volt::RepRPCData& rpcData);
Nexus::Packet SerializeRPCPacket(std::string in_project, std::string in_class, std::string in_method, Nexus::TYPE::REP_ID in_netId, Nexus::TYPE::RPC_ID in_id = 0);
Volt::RepRPCData CreateRPCData(std::string in_project, std::string in_class, std::string in_method, Nexus::TYPE::REP_ID in_netId, Nexus::TYPE::RPC_ID in_id = 0);
#pragma endregion

#pragma region Variable
Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::RepVariableData& data);
Nexus::Packet& operator>(Nexus::Packet& packet, Volt::RepVariableData& varData);
Nexus::Packet SerializeVariablePacket(Volt::RepVariable& variable, Nexus::TYPE::REP_ID repId);
#pragma endregion

#pragma region Prefab
Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::RepPrefabData& data);
Nexus::Packet& operator>(Nexus::Packet& packet, Volt::RepPrefabData& data);
void RecursiveOwnerShipControll(entt::entity in_id, const Volt::RepPrefabData& data);
void RecursiveHandleMono(entt::entity entId, Nexus::TYPE::REP_ID owner, Nexus::TYPE::REP_ID& varId, Nexus::ReplicationRegisty* registry = nullptr, bool manageInstances = true);
bool ConstructPrefab(const Volt::RepPrefabData& data, Nexus::ReplicationRegisty& registry);
Volt::RepPrefabData CreatePrefabData(Nexus::TYPE::REP_ID in_repId, Nexus::TYPE::CLIENT_ID in_ownerId, Volt::AssetHandle in_handle);
#pragma endregion

#pragma region NetEvent
Nexus::Packet& operator<(Nexus::Packet& packet, const Volt::NetEvent& netEvent);
Nexus::Packet& operator>(Nexus::Packet& packet, Volt::NetEvent& netEvent);
Nexus::Packet SerializeNetEventPacket(Volt::NetEvent in_event);
#pragma endregion
