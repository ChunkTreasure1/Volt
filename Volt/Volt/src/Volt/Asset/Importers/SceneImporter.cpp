#include "vtpch.h"
#include "SceneImporter.h"
//#include "SceneImporter.h"
//
//#include "Volt/Scene/Scene.h"
//
//#include "Volt/Utility/SerializationMacros.h"
//#include "Volt/Utility/YAMLSerializationHelpers.h"
//
//#include "Volt/Asset/Prefab.h"
//#include "Volt/Asset/AssetManager.h"
//
//#include "Volt/Components/Components.h"
//#include "Volt/Project/ProjectManager.h"
//
//#include "Volt/Core/Profiling.h"
//#include "Volt/Utility/FileSystem.h"
//
//#include "Volt/Scene/Entity.h"
//#include "Volt/Scripting/Mono/MonoScriptEngine.h"
//#include "Volt/Scripting/Mono/MonoScriptClass.h"
//
//#include "Volt/Utility/PackUtility.h"
//
//#include <GraphKey/Graph.h>
//#include <GraphKey/Node.h>
//#include <GraphKey/Registry.h>
//#include <GraphKey/Nodes/ParameterNodes.h>
//#include <GraphKey/Nodes/CustomEventNode.h>
//
//#include <Volt/Core/BinarySerializer.h>
//#include <stdint.h>
//
//#include <yaml-cpp/yaml.h>
//
//namespace Volt
//{
//	template<typename T>
//	inline static void DeserializeVector(uint8_t* data, uint32_t offset, YAML::Node node, T defaultValue)
//	{
//		std::vector<T>& vector = *(std::vector<T>*) & data[offset];
//		for (const auto& vecNode : node)
//		{
//			VT_DESERIALIZE_PROPERTY(value, vector.emplace_back(), vecNode, defaultValue);
//		}
//	}
//
//	template<typename T>
//	inline static void SerializeVector(uint8_t* data, uint32_t offset, YAML::Emitter& out)
//	{
//		std::vector<T>& items = *(std::vector<T>*) & data[offset];
//		for (const auto& item : items)
//		{
//			out << YAML::BeginMap;
//			VT_SERIALIZE_PROPERTY(value, item, out);
//			out << YAML::EndMap;
//		}
//	}
//
//	template<typename T>
//	inline constexpr std::type_index GetTypeIndex()
//	{
//		return std::type_index(typeid(T));
//	}
//
//	SceneImporter::SceneImporter()
//	{
//		// Serialize
//		{
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Bool] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(bool*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Int] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(int32_t*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::UInt] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(uint32_t*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Short] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(int16_t*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::UShort] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(uint16_t*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Char] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(int8_t*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::UChar] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(uint8_t*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Float] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(float*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Double] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(double*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Vector2] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(glm::vec2*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Vector3] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(glm::vec3*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Vector4] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(glm::vec4*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Quaternion] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(glm::quat*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::String] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(std::string*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Int64] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(int64_t*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::UInt64] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(uint64_t*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::AssetHandle] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(AssetHandle*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Color3] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(glm::vec3*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Color4] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(glm::vec4*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Directory] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(std::filesystem::path*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Path] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(std::filesystem::path*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::GUID] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(WireGUID*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::EntityId] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(uint32_t*)&componentData[offset], out); };
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Enum] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, YAML::Emitter& out) { VT_SERIALIZE_PROPERTY(data, *(uint32_t*)&componentData[offset], out); };
//
//			myPropertySerializer[Wire::ComponentRegistry::PropertyType::Vector] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType vectorType, YAML::Emitter& out)
//			{
//				out << YAML::Key << "data" << YAML::BeginSeq;
//				switch (vectorType)
//				{
//					case Wire::ComponentRegistry::PropertyType::Bool: SerializeVector<bool>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Int: SerializeVector<int32_t>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::UInt: SerializeVector<uint32_t>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Short: SerializeVector<int16_t>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::UShort: SerializeVector<uint16_t>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Char: SerializeVector<int8_t>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::UChar: SerializeVector<uint8_t>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Float: SerializeVector<float>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Double: SerializeVector<double>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Vector2: SerializeVector<glm::vec2>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Vector3: SerializeVector<glm::vec3>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Vector4: SerializeVector<glm::vec4>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Quaternion: SerializeVector<glm::quat>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::String: SerializeVector<std::string>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Int64: SerializeVector<int64_t>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::UInt64: SerializeVector<uint64_t>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::AssetHandle: SerializeVector<AssetHandle>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Color3: SerializeVector<glm::vec3>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Color4: SerializeVector<glm::vec4>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Directory: SerializeVector<std::filesystem::path>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Path: SerializeVector<std::filesystem::path>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::GUID: SerializeVector<WireGUID>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::EntityId: SerializeVector<entt::entity>(componentData, offset, out); break;
//					case Wire::ComponentRegistry::PropertyType::Enum: SerializeVector<uint32_t>(componentData, offset, out); break;
//				}
//
//				out << YAML::EndSeq;
//			};
//		}
//
//		// Deserialize
//		{
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Bool] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				bool input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, false);
//				memcpy_s(&componentData[offset], sizeof(bool), &input, sizeof(bool));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Int] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				int32_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, 0);
//				memcpy_s(&componentData[offset], sizeof(int32_t), &input, sizeof(int32_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::UInt] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				uint32_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, 0u);
//				memcpy_s(&componentData[offset], sizeof(uint32_t), &input, sizeof(uint32_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Short] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				int16_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, int16_t(0));
//				memcpy_s(&componentData[offset], sizeof(int16_t), &input, sizeof(int16_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::UShort] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				uint16_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, uint16_t(0));
//				memcpy_s(&componentData[offset], sizeof(uint16_t), &input, sizeof(uint16_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Char] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				int8_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, int8_t(0));
//				memcpy_s(&componentData[offset], sizeof(int8_t), &input, sizeof(int8_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::UChar] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				uint8_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, uint8_t(0));
//				memcpy_s(&componentData[offset], sizeof(uint8_t), &input, sizeof(uint8_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Float] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				float input;
//
//				VT_DESERIALIZE_PROPERTY(data, input, node, 0.f);
//				memcpy_s(&componentData[offset], sizeof(float), &input, sizeof(float));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Double] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				double input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, 0.0);
//				memcpy_s(&componentData[offset], sizeof(double), &input, sizeof(double));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Vector2] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				glm::vec2 input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, glm::vec2{ 0.f });
//				memcpy_s(&componentData[offset], sizeof(glm::vec2), &input, sizeof(glm::vec2));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Vector3] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				glm::vec3 input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, glm::vec3{ 0.f });
//				memcpy_s(&componentData[offset], sizeof(glm::vec3), &input, sizeof(glm::vec3));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Vector4] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				glm::vec4 input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, glm::vec4{ 0.f });
//				memcpy_s(&componentData[offset], sizeof(glm::vec4), &input, sizeof(glm::vec4));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Quaternion] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				glm::quat input{};
//				VT_DESERIALIZE_PROPERTY(data, input, node, glm::quat{});
//				memcpy_s(&componentData[offset], sizeof(glm::quat), &input, sizeof(glm::quat));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::String] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				std::string input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, std::string(""));
//
//				(*(std::string*)&componentData[offset]) = input;
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Int64] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				int64_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, int64_t(0));
//				memcpy_s(&componentData[offset], sizeof(int64_t), &input, sizeof(int64_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::UInt64] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				uint64_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, uint64_t(0));
//				memcpy_s(&componentData[offset], sizeof(uint64_t), &input, sizeof(uint64_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::AssetHandle] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				AssetHandle input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, AssetHandle(0));
//				memcpy_s(&componentData[offset], sizeof(AssetHandle), &input, sizeof(AssetHandle));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Color3] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				glm::vec3 input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, glm::vec3{ 0.f });
//				memcpy_s(&componentData[offset], sizeof(glm::vec3), &input, sizeof(glm::vec3));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Color4] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				glm::vec4 input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, glm::vec4{ 0.f });
//				memcpy_s(&componentData[offset], sizeof(glm::vec4), &input, sizeof(glm::vec4));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Directory] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				std::filesystem::path input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, std::filesystem::path(""));
//				memcpy_s(&componentData[offset], sizeof(std::filesystem::path), &input, sizeof(std::filesystem::path));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Path] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				std::filesystem::path input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, std::filesystem::path(""));
//				memcpy_s(&componentData[offset], sizeof(std::filesystem::path), &input, sizeof(std::filesystem::path));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::EntityId] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				uint32_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, uint32_t(0));
//				memcpy_s(&componentData[offset], sizeof(uint32_t), &input, sizeof(uint32_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::GUID] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				WireGUID input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, WireGUID::Null());
//				memcpy_s(&componentData[offset], sizeof(WireGUID), &input, sizeof(WireGUID));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Enum] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType, const YAML::Node& node)
//			{
//				uint32_t input;
//				VT_DESERIALIZE_PROPERTY(data, input, node, 0u);
//				memcpy_s(&componentData[offset], sizeof(uint32_t), &input, sizeof(uint32_t));
//			};
//
//			myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Vector] = [](uint8_t* componentData, uint32_t offset, Wire::ComponentRegistry::PropertyType vectorType, const YAML::Node& node)
//			{
//				if (!node["data"])
//				{
//					return;
//				}
//
//				switch (vectorType)
//				{
//					case Wire::ComponentRegistry::PropertyType::Bool: DeserializeVector<bool>(componentData, offset, node["data"], false); break;
//					case Wire::ComponentRegistry::PropertyType::Int: DeserializeVector<int32_t>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::UInt: DeserializeVector<uint32_t>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Short: DeserializeVector<int16_t>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::UShort: DeserializeVector<uint16_t>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Char: DeserializeVector<int8_t>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::UChar: DeserializeVector<uint8_t>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Float: DeserializeVector<float>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Double: DeserializeVector<double>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Vector2: DeserializeVector<glm::vec2>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Vector3: DeserializeVector<glm::vec3>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Vector4: DeserializeVector<glm::vec4>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Quaternion: DeserializeVector<glm::quat>(componentData, offset, node["data"], glm::quat{ 1.f, 0.f, 0.f, 0.f }); break;
//					case Wire::ComponentRegistry::PropertyType::String: DeserializeVector<std::string>(componentData, offset, node["data"], "Null"); break;
//					case Wire::ComponentRegistry::PropertyType::Int64: DeserializeVector<int64_t>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::UInt64: DeserializeVector<uint64_t>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::AssetHandle: DeserializeVector<AssetHandle>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Color3: DeserializeVector<glm::vec3>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Color4: DeserializeVector<glm::vec4>(componentData, offset, node["data"], 0); break;
//					case Wire::ComponentRegistry::PropertyType::Directory: DeserializeVector<std::filesystem::path>(componentData, offset, node["data"], "Null"); break;
//					case Wire::ComponentRegistry::PropertyType::Path: DeserializeVector<std::filesystem::path>(componentData, offset, node["data"], "Null"); break;
//					case Wire::ComponentRegistry::PropertyType::GUID: DeserializeVector<WireGUID>(componentData, offset, node["data"], WireGUID::Null()); break;
//					case Wire::ComponentRegistry::PropertyType::EntityId: DeserializeVector<entt::entity>(componentData, offset, node["data"], Wire::NullID); break;
//					case Wire::ComponentRegistry::PropertyType::Enum: DeserializeVector<uint32_t>(componentData, offset, node["data"], 0); break;
//				}
//			};
//		}
//
//
//		// Type index serialize
//		{
//			myTypeIndexSerializeMap[GetTypeIndex<bool>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<bool>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Bool]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Bool, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<int32_t>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<int32_t>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Int]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Int, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<uint32_t>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<uint32_t>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::UInt]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::UInt, out);
//			};
//			myTypeIndexSerializeMap[GetTypeIndex<int16_t>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<int16_t>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Short]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Short, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<uint16_t>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<uint16_t>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::UShort]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::UShort, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<int8_t>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<int8_t>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Char]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Char, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<uint8_t>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<uint8_t>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::UChar]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::UChar, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<float>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<float>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Float]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Float, out);
//			};
//			myTypeIndexSerializeMap[GetTypeIndex<double>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<double>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Double]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Double, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<glm::vec2>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<glm::vec2>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Vector2]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Vector2, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<glm::vec3>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<glm::vec3>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Vector3]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Vector3, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<glm::vec4>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<glm::vec4>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Vector4]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Vector4, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<glm::quat>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<glm::quat>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Quaternion]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Quaternion, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<std::string>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<std::string>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::String]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::String, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<int64_t>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<int64_t>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Int64]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Int64, out);
//			};
//			myTypeIndexSerializeMap[GetTypeIndex<uint64_t>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<uint64_t>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::UInt64]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::UInt64, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<AssetHandle>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<AssetHandle>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::AssetHandle]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::AssetHandle, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<std::filesystem::path>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<std::filesystem::path>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::Path]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::Path, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<WireGUID>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto var = std::any_cast<WireGUID>(data);
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::GUID]((uint8_t*)&var, 0, Wire::ComponentRegistry::PropertyType::GUID, out);
//			};
//
//			myTypeIndexSerializeMap[GetTypeIndex<Volt::Entity>()] = [](const std::any& data, YAML::Emitter& out)
//			{
//				auto ent = std::any_cast<Volt::Entity>(data);
//				auto id = ent.GetId();
//
//				myPropertySerializer[Wire::ComponentRegistry::PropertyType::EntityId]((uint8_t*)&id, 0, Wire::ComponentRegistry::PropertyType::EntityId, out);
//			};
//		}
//
//		// Type index deserialize
//		{
//			myTypeIndexDeserializeMap[GetTypeIndex<bool>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = bool{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Bool]((uint8_t*)&std::any_cast<bool&>(data), 0, Wire::ComponentRegistry::PropertyType::Bool, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<int32_t>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = int32_t{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Int]((uint8_t*)&std::any_cast<int32_t&>(data), 0, Wire::ComponentRegistry::PropertyType::Int, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<uint32_t>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = uint32_t{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::UInt]((uint8_t*)&std::any_cast<uint32_t&>(data), 0, Wire::ComponentRegistry::PropertyType::UInt, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<int16_t>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = int16_t{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Short]((uint8_t*)&std::any_cast<int16_t&>(data), 0, Wire::ComponentRegistry::PropertyType::Short, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<uint16_t>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = uint16_t{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::UShort]((uint8_t*)&std::any_cast<uint16_t&>(data), 0, Wire::ComponentRegistry::PropertyType::UShort, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<int8_t>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = int8_t{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Char]((uint8_t*)&std::any_cast<int8_t&>(data), 0, Wire::ComponentRegistry::PropertyType::Char, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<uint8_t>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = uint8_t{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::UChar]((uint8_t*)&std::any_cast<uint8_t&>(data), 0, Wire::ComponentRegistry::PropertyType::UChar, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<float>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = float{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Float]((uint8_t*)&std::any_cast<float&>(data), 0, Wire::ComponentRegistry::PropertyType::Float, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<double>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = double{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Double]((uint8_t*)&std::any_cast<double&>(data), 0, Wire::ComponentRegistry::PropertyType::Double, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<glm::vec2>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = glm::vec2{ 0.f };
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Vector2]((uint8_t*)&std::any_cast<glm::vec2&>(data), 0, Wire::ComponentRegistry::PropertyType::Vector2, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<glm::vec3>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = glm::vec3{ 0.f };
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Vector3]((uint8_t*)&std::any_cast<glm::vec3&>(data), 0, Wire::ComponentRegistry::PropertyType::Vector3, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<glm::vec4>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = glm::vec4{ 0.f };
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Vector4]((uint8_t*)&std::any_cast<glm::vec4&>(data), 0, Wire::ComponentRegistry::PropertyType::Vector4, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<glm::quat>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = glm::quat{ 1.f, 0.f, 0.f, 0.f };
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Quaternion]((uint8_t*)&std::any_cast<glm::quat&>(data), 0, Wire::ComponentRegistry::PropertyType::Quaternion, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<std::string>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = std::string{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::String]((uint8_t*)&std::any_cast<std::string&>(data), 0, Wire::ComponentRegistry::PropertyType::String, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<int64_t>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = int64_t{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Int64]((uint8_t*)&std::any_cast<int64_t&>(data), 0, Wire::ComponentRegistry::PropertyType::Int64, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<uint64_t>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = uint64_t{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::UInt64]((uint8_t*)&std::any_cast<uint64_t&>(data), 0, Wire::ComponentRegistry::PropertyType::UInt64, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<AssetHandle>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = AssetHandle{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::AssetHandle]((uint8_t*)&std::any_cast<AssetHandle&>(data), 0, Wire::ComponentRegistry::PropertyType::AssetHandle, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<std::filesystem::path>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = std::filesystem::path{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::Path]((uint8_t*)&std::any_cast<std::filesystem::path&>(data), 0, Wire::ComponentRegistry::PropertyType::Path, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<WireGUID>()] = [](std::any& data, const YAML::Node& node)
//			{
//				data = WireGUID{};
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::GUID]((uint8_t*)&std::any_cast<WireGUID&>(data), 0, Wire::ComponentRegistry::PropertyType::GUID, node);
//			};
//
//			myTypeIndexDeserializeMap[GetTypeIndex<Volt::Entity>()] = [](std::any& data, const YAML::Node& node)
//			{
//				entt::entity id = 0;
//
//				myPropertyDeserializer[Wire::ComponentRegistry::PropertyType::EntityId]((uint8_t*)&id, 0, Wire::ComponentRegistry::PropertyType::EntityId, node);
//				data = Volt::Entity{ id, nullptr };
//			};
//		}
//	}
//
//	SceneImporter::~SceneImporter()
//	{
//	}
//
//	bool SceneImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
//	{
//		VT_PROFILE_FUNCTION();
//
//		asset = CreateRef<Scene>();
//		Ref<Scene> scene = reinterpret_pointer_cast<Scene>(asset);
//
//		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
//
//		if (!std::filesystem::exists(filePath)) [[unlikely]]
//		{
//			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
//			asset->SetFlag(AssetFlag::Missing, true);
//			return false;
//		}
//
//		std::ifstream file(filePath);
//		if (!file.is_open()) [[unlikely]]
//		{
//			VT_CORE_ERROR("Failed to open file {0}!", metadata.filePath);
//			asset->SetFlag(AssetFlag::Invalid, true);
//			return false;
//		}
//
//		const std::filesystem::path& scenePath = filePath;
//		std::filesystem::path folderPath = scenePath.parent_path();
//		std::filesystem::path entitiesFolderPath = folderPath / "Entities";
//
//		std::stringstream sstream;
//		sstream << file.rdbuf();
//		file.close();
//
//		YAML::Node root;
//
//		try
//		{
//			root = YAML::Load(sstream.str());
//		}
//		catch (std::exception& e)
//		{
//			VT_CORE_ERROR("{0} contains invalid YAML with error {1}! Please correct it!", metadata.filePath, e.what());
//			asset->SetFlag(AssetFlag::Invalid, true);
//			return false;
//		}
//
//		YAML::Node sceneNode = root["Scene"];
//
//		VT_DESERIALIZE_PROPERTY(name, scene->myName, sceneNode, std::string("New Scene"));
//
//		LoadLayers(metadata, scene, folderPath);
//		scene->SortScene();
//		return true;
//	}
//
//	void SceneImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
//	{
//		VT_PROFILE_FUNCTION();
//		const Ref<Scene> scene = std::reinterpret_pointer_cast<Scene>(asset);
//
//		std::filesystem::path folderPath = AssetManager::GetFilesystemPath(metadata.filePath);
//		if (!std::filesystem::is_directory(folderPath))
//		{
//			folderPath = folderPath.parent_path();
//		}
//
//		std::filesystem::path scenePath = folderPath / (metadata.filePath.stem().string() + ".vtscene");
//		std::filesystem::path entitiesFolderPath = folderPath / "Entities";
//
//		if (!std::filesystem::exists(folderPath))
//		{
//			std::filesystem::create_directories(folderPath);
//		}
//
//		YAML::Emitter out;
//		out << YAML::BeginMap;
//		out << YAML::Key << "Scene" << YAML::Value;
//		{
//			out << YAML::BeginMap;
//			VT_SERIALIZE_PROPERTY(name, metadata.filePath.stem(), out);
//			out << YAML::EndMap;
//		}
//		out << YAML::EndMap;
//
//		std::ofstream file;
//		file.open(scenePath);
//		file << out.c_str();
//		file.close();
//
//		SaveLayers(metadata, scene, folderPath);
//	}
//
//	void SceneImporter::SaveLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
//	{
//		std::filesystem::path layerFolderPath = sceneDirectory / "Layers";
//		if (!std::filesystem::exists(layerFolderPath))
//		{
//			std::filesystem::create_directories(layerFolderPath);
//		}
//
//		for (const auto& layer : scene->GetLayers())
//		{
//			const auto layerPath = layerFolderPath / ("layer_" + std::to_string(layer.id) + ".vtlayer");
//
//			YAML::Emitter out;
//			out << YAML::BeginMap;
//			out << YAML::Key << "Layer" << YAML::Value;
//			{
//				out << YAML::BeginMap;
//				VT_SERIALIZE_PROPERTY(name, layer.name, out);
//				VT_SERIALIZE_PROPERTY(id, layer.id, out);
//				VT_SERIALIZE_PROPERTY(visible, layer.visible, out);
//				VT_SERIALIZE_PROPERTY(locked, layer.locked, out);
//
//				out << YAML::Key << "Entities" << YAML::BeginSeq;
//
//				for (const auto entity : scene->GetRegistry().GetAllEntities())
//				{
//					if (!scene->GetRegistry().HasComponent<EntityDataComponent>(entity))
//					{
//						continue;
//					}
//
//					const uint32_t layerId = scene->GetRegistry().GetComponent<EntityDataComponent>(entity).layerId;
//					if (layerId != layer.id)
//					{
//						continue;
//					}
//
//					SerializeEntityLayer(entity, scene->GetRegistry(), out, metadata, scene);
//				}
//
//				out << YAML::EndSeq;
//				out << YAML::EndMap;
//			}
//			out << YAML::EndMap;
//
//			std::ofstream file;
//			file.open(layerPath);
//			file << out.c_str();
//			file.close();
//		}
//	}
//
//	void SceneImporter::LoadLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
//	{
//		VT_PROFILE_FUNCTION();
//
//		std::filesystem::path layersFolderPath = sceneDirectory / "Layers";
//		if (!std::filesystem::exists(layersFolderPath))
//		{
//			return;
//		}
//
//		std::vector<std::filesystem::path> layerPaths;
//
//		for (const auto& it : std::filesystem::directory_iterator(layersFolderPath))
//		{
//			if (!it.is_directory() && it.path().extension().string() == ".vtlayer")
//			{
//				layerPaths.emplace_back(it.path());
//			}
//		}
//
//		// Divide paths into threads and load
//		auto threadFunc = [&](const std::string& name, const std::filesystem::path& path, Wire::Registry& registry, SceneLayer& sceneLayer)
//		{
//			VT_PROFILE_THREAD(name.c_str());
//
//			std::ifstream file(path);
//			std::stringstream sstream;
//			sstream << file.rdbuf();
//			file.close();
//
//			YAML::Node root;
//
//			try
//			{
//				root = YAML::Load(sstream.str());
//			}
//			catch (std::exception& e)
//			{
//				VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", path, e.what());
//				return;
//			}
//
//			YAML::Node layerNode = root["Layer"];
//
//			VT_DESERIALIZE_PROPERTY(name, sceneLayer.name, layerNode, std::string("Null"));
//			VT_DESERIALIZE_PROPERTY(id, sceneLayer.id, layerNode, 0u);
//			VT_DESERIALIZE_PROPERTY(visible, sceneLayer.visible, layerNode, true);
//			VT_DESERIALIZE_PROPERTY(locked, sceneLayer.locked, layerNode, true);
//
//			YAML::Node entitiesNode = layerNode["Entities"];
//			for (const auto& entityNode : entitiesNode)
//			{
//				DeserializeEntityLayer(registry, metadata, scene, entityNode);
//			}
//		};
//
//		std::vector<Wire::Registry> threadRegistries{ layerPaths.size() };
//		std::vector<SceneLayer> threadSceneLayers{ layerPaths.size() };
//		std::vector<std::thread> threads;
//
//		for (uint32_t i = 0; i < (uint32_t)layerPaths.size(); i++)
//		{
//			const std::string name = "Import Worker#" + std::to_string(i);
//			threads.emplace_back(threadFunc, name, layerPaths.at(i), std::reference_wrapper(threadRegistries.at(i)), std::reference_wrapper(threadSceneLayers.at(i)));
//		}
//
//		for (auto& thread : threads)
//		{
//			thread.join();
//		}
//
//		std::vector<entt::entity> dirtyPrefabs{};
//		for (auto& registry : threadRegistries)
//		{
//			registry.ForEach<PrefabComponent>([&](entt::entity id, PrefabComponent& prefabComp)
//			{
//				if (prefabComp.isDirty)
//				{
//					dirtyPrefabs.emplace_back(id);
//				}
//			});
//
//			for (const auto& entity : registry.GetAllEntities())
//			{
//				if (!registry.HasComponent<Volt::TagComponent>(entity) || !registry.HasComponent<Volt::TransformComponent>(entity))
//				{
//					continue;
//				}
//
//				entt::entity newEntityId = scene->myRegistry.AddEntity(entity);
//				Entity::Copy(registry, scene->myRegistry, scene->GetScriptFieldCache(), scene->GetScriptFieldCache(), entity, newEntityId);
//			}
//		}
//
//		for (const auto& id : dirtyPrefabs)
//		{
//			auto& prefabComp = scene->myRegistry.GetComponent<PrefabComponent>(id);
//			Prefab::UpdateEntity(scene.get(), id, prefabComp.prefabAsset);
//			prefabComp.isDirty = false;
//		}
//
//		scene->myRegistry.ForEach<VisualScriptingComponent>([&](entt::entity, VisualScriptingComponent& comp)
//		{
//			if (comp.graph)
//			{
//				for (const auto& n : comp.graph->GetNodes())
//				{
//					for (auto& i : n->inputs)
//					{
//						if (!comp.graph->IsAttributeLinked(i.id) && i.data.has_value() && i.data.type() == typeid(Volt::Entity))
//						{
//							Volt::Entity entity = std::any_cast<Volt::Entity>(i.data);
//							i.data = Volt::Entity{ entity.GetId(), scene.get() };
//						}
//					}
//
//					for (auto& o : n->outputs)
//					{
//						if (o.data.has_value() && o.data.type() == typeid(Volt::Entity))
//						{
//							Volt::Entity entity = std::any_cast<Volt::Entity>(o.data);
//							o.data = Volt::Entity{ entity.GetId(), scene.get() };
//						}
//					}
//				}
//			}
//		});
//
//		// Remove all child that do not exist
//		scene->myRegistry.ForEach<RelationshipComponent>([&](entt::entity, RelationshipComponent& comp)
//		{
//			for (int32_t i = (int32_t)comp.Children.size() - 1; i > 0; i--)
//			{
//				if (!scene->myRegistry.Exists(comp.Children.at(i)))
//				{
//					comp.Children.erase(comp.Children.begin() + i);
//				}
//			}
//		});
//
//		uint32_t maxSceneLayer = 0;
//		for (const auto& layer : threadSceneLayers)
//		{
//			maxSceneLayer = std::max(maxSceneLayer, layer.id);
//		}
//
//		scene->myLastLayerId = maxSceneLayer + 1;
//		scene->mySceneLayers = threadSceneLayers;
//	}
//
//	bool SceneImporter::CheckComponentDataMatch(const uint8_t* buf1, const uint8_t* buf2, Wire::ComponentRegistry::ComponentProperty prop) const
//	{
//		switch (prop.type)
//		{
//			case Wire::ComponentRegistry::PropertyType::Bool: return ((*(bool*)&buf1[prop.offset]) == (*(bool*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Int: return ((*(int32_t*)&buf1[prop.offset]) == (*(int32_t*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::UInt: return ((*(uint32_t*)&buf1[prop.offset]) == (*(uint32_t*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Short: return ((*(int16_t*)&buf1[prop.offset]) == (*(int16_t*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::UShort: return ((*(uint16_t*)&buf1[prop.offset]) == (*(uint16_t*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Char: return ((*(int8_t*)&buf1[prop.offset]) == (*(int8_t*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::UChar: return ((*(uint8_t*)&buf1[prop.offset]) == (*(uint8_t*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Float: return ((*(float*)&buf1[prop.offset]) == (*(float*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Double: return ((*(double*)&buf1[prop.offset]) == (*(double*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Vector2: return ((*(glm::vec2*)&buf1[prop.offset]) == (*(glm::vec2*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Vector3: return ((*(glm::vec3*)&buf1[prop.offset]) == (*(glm::vec3*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Vector4: return ((*(glm::vec4*)&buf1[prop.offset]) == (*(glm::vec4*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Quaternion: return ((*(glm::quat*)&buf1[prop.offset]) == (*(glm::quat*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::String: return ((*(std::string*)&buf1[prop.offset]) == (*(std::string*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Int64: return ((*(int64_t*)&buf1[prop.offset]) == (*(int64_t*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::UInt64: return ((*(uint64_t*)&buf1[prop.offset]) == (*(uint64_t*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::AssetHandle: return ((*(AssetHandle*)&buf1[prop.offset]) == (*(AssetHandle*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Color3: return ((*(glm::vec3*)&buf1[prop.offset]) == (*(glm::vec3*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Color4: return ((*(glm::vec4*)&buf1[prop.offset]) == (*(glm::vec4*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Directory: return ((*(std::filesystem::path*)&buf1[prop.offset]) == (*(std::filesystem::path*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Path: return ((*(std::filesystem::path*)&buf1[prop.offset]) == (*(std::filesystem::path*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::EntityId: return ((*(entt::entity*)&buf1[prop.offset]) == (*(entt::entity*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::GUID: return ((*(WireGUID*)&buf1[prop.offset]) == (*(WireGUID*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Enum: return ((*(uint32_t*)&buf1[prop.offset]) == (*(uint32_t*)&buf2[prop.offset])) ? true : false;
//			case Wire::ComponentRegistry::PropertyType::Vector:
//			{
//				switch (prop.vectorType)
//				{
//					case Wire::ComponentRegistry::PropertyType::Bool: return ((*(std::vector<bool>*) & buf1[prop.offset]) == (*(std::vector<bool>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Int: return ((*(std::vector<int32_t>*) & buf1[prop.offset]) == (*(std::vector<int32_t>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::UInt: return ((*(std::vector<uint32_t>*) & buf1[prop.offset]) == (*(std::vector<uint32_t>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Short: return ((*(std::vector<int16_t>*) & buf1[prop.offset]) == (*(std::vector<int16_t>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::UShort: return ((*(std::vector<uint16_t>*) & buf1[prop.offset]) == (*(std::vector<uint16_t>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Char: return ((*(std::vector<int8_t>*) & buf1[prop.offset]) == (*(std::vector<int8_t>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::UChar: return ((*(std::vector<uint8_t>*) & buf1[prop.offset]) == (*(std::vector<uint8_t>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Float: return ((*(std::vector<float>*) & buf1[prop.offset]) == (*(std::vector<float>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Double: return ((*(std::vector<double>*) & buf1[prop.offset]) == (*(std::vector<double>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Vector2: return ((*(std::vector<glm::vec2>*) & buf1[prop.offset]) == (*(std::vector<glm::vec2>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Vector3: return ((*(std::vector<glm::vec3>*) & buf1[prop.offset]) == (*(std::vector<glm::vec3>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Vector4: return ((*(std::vector<glm::vec4>*) & buf1[prop.offset]) == (*(std::vector<glm::vec4>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Quaternion: return ((*(std::vector<glm::quat>*) & buf1[prop.offset]) == (*(std::vector<glm::quat>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::String: return ((*(std::vector<std::string>*) & buf1[prop.offset]) == (*(std::vector<std::string>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Int64: return ((*(std::vector<int64_t>*) & buf1[prop.offset]) == (*(std::vector<int64_t>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::UInt64: return ((*(std::vector<uint64_t>*) & buf1[prop.offset]) == (*(std::vector<uint64_t>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::AssetHandle: return ((*(std::vector<AssetHandle>*) & buf1[prop.offset]) == (*(std::vector<AssetHandle>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Color4: return ((*(std::vector<glm::vec2>*) & buf1[prop.offset]) == (*(std::vector<glm::vec2>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Color3: return ((*(std::vector<glm::vec3>*) & buf1[prop.offset]) == (*(std::vector<glm::vec3>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Directory: return ((*(std::vector<glm::vec4>*) & buf1[prop.offset]) == (*(std::vector<glm::vec4>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::EntityId: return ((*(std::vector<entt::entity>*) & buf1[prop.offset]) == (*(std::vector<entt::entity>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::GUID: return ((*(std::vector<WireGUID>*) & buf1[prop.offset]) == (*(std::vector<WireGUID>*) & buf2[prop.offset])) ? true : false;
//					case Wire::ComponentRegistry::PropertyType::Enum: return ((*(uint32_t*)&buf1[prop.offset]) == (*(uint32_t*)&buf2[prop.offset])) ? true : false;
//				}
//				break;
//			}
//			default: return false;
//		}
//
//		return false;
//	}
//
//	void SceneImporter::SerializeMono(entt::entity id, const MonoScriptFieldCache& scriptFieldCache, const Wire::Registry& registry, YAML::Emitter& out)
//	{
//		out << YAML::Key << "MonoScripts" << YAML::BeginSeq;
//		{
//			auto msComp = (MonoScriptComponent*)registry.GetComponentPtr(MonoScriptComponent::comp_guid, id);
//			for (uint32_t i = 0; i < msComp->scriptIds.size(); ++i)
//			{
//				out << YAML::BeginMap;
//				{
//					out << YAML::Key << "ScriptEntry" << YAML::Value;
//					{
//						out << YAML::BeginMap;
//						VT_SERIALIZE_PROPERTY(name, msComp->scriptNames[i], out);
//						VT_SERIALIZE_PROPERTY(id, msComp->scriptIds[i], out);
//
//						if (scriptFieldCache.GetCache().contains(msComp->scriptIds[i]))
//						{
//							out << YAML::Key << "properties" << YAML::BeginSeq;
//							auto fieldMap = scriptFieldCache.GetCache().at(msComp->scriptIds[i]);
//							for (auto& [name, value] : fieldMap)
//							{
//								auto propType = MonoScriptClass::MonoFieldTypeToWirePropType(value->field.type);
//
//								out << YAML::BeginMap;
//								VT_SERIALIZE_PROPERTY(type, propType, out);
//								VT_SERIALIZE_PROPERTY(name, name, out);
//								VT_SERIALIZE_PROPERTY(enumName, value->field.enumName, out);
//
//								switch (value->field.type)
//								{
//									case MonoFieldType::String:
//									{
//										auto cstr = value->data.As<const char>();
//										std::string str(cstr);
//										VT_SERIALIZE_PROPERTY(data, str, out);
//										break;
//									}
//
//									default:
//									{
//										if (propType != Wire::ComponentRegistry::PropertyType::Unknown)
//										{
//											auto data = value->data.As<uint8_t>();
//											GetPropertySerializers()[propType](data, 0, Wire::ComponentRegistry::PropertyType::Unknown, out);
//											break;
//										}
//									}
//								}
//
//								out << YAML::EndMap;
//							}
//							out << YAML::EndSeq;
//						}
//
//						out << YAML::EndMap;
//					}
//				}
//				out << YAML::EndMap;
//			}
//		}
//		out << YAML::EndSeq;
//	}
//
//	void SceneImporter::DeserializeMono(entt::entity, MonoScriptFieldCache& scriptFieldCache, const YAML::Node& node)
//	{
//		for (const auto& n : node["MonoScripts"])
//		{
//			std::string scriptName;
//			UUID scriptId;
//			VT_DESERIALIZE_PROPERTY(name, scriptName, n["ScriptEntry"], std::string("Null"));
//			VT_DESERIALIZE_PROPERTY(id, scriptId, n["ScriptEntry"], UUID(0));
//
//			auto& savedFieldMap = scriptFieldCache.GetCache()[scriptId];
//
//			for (const auto& m : n["ScriptEntry"]["properties"])
//			{
//				Wire::ComponentRegistry::PropertyType propType;
//				std::string propName;
//				std::string enumName;
//				uint8_t propData[16];
//
//				VT_DESERIALIZE_PROPERTY(type, propType, m, Wire::ComponentRegistry::PropertyType::Unknown);
//				VT_DESERIALIZE_PROPERTY(name, propName, m, std::string("Null"));
//				VT_DESERIALIZE_PROPERTY(enumName, enumName, m, std::string(""));
//
//				switch (propType)
//				{
//					case Wire::ComponentRegistry::PropertyType::String:
//					{
//						break;
//					}
//
//					default:
//					{
//						if (propType != Wire::ComponentRegistry::PropertyType::Unknown)
//						{
//							GetPropertyDeserializers().at(propType)(&propData[0], 0, Wire::ComponentRegistry::PropertyType::Unknown, m);
//						}
//						break;
//					}
//				}
//
//				savedFieldMap[propName] = CreateRef<MonoScriptFieldInstance>();
//				savedFieldMap.at(propName)->field.enumName = enumName;
//
//				MonoFieldType fieldType = MonoScriptClass::WirePropTypeToMonoFieldType(propType);
//
//				if (fieldType == MonoFieldType::Asset)
//				{
//					auto handle = *(AssetHandle*)&propData[0];
//
//					switch (AssetManager::GetAssetTypeFromHandle(handle))
//					{
//						case AssetType::Animation: fieldType = MonoFieldType::Animation; break;
//						case AssetType::Prefab: fieldType = MonoFieldType::Prefab; break;
//						case AssetType::Scene: fieldType = MonoFieldType::Scene; break;
//						case AssetType::Mesh: fieldType = MonoFieldType::Mesh; break;
//						case AssetType::Font: fieldType = MonoFieldType::Font; break;
//						case AssetType::Material: fieldType = MonoFieldType::Material; break;
//						case AssetType::Texture: fieldType = MonoFieldType::Texture; break;
//						case AssetType::PostProcessingMaterial: fieldType = MonoFieldType::PostProcessingMaterial; break;
//						case AssetType::Video: fieldType = MonoFieldType::Video; break;
//						case AssetType::AnimationGraph: fieldType = MonoFieldType::AnimationGraph; break;
//						default: fieldType = MonoFieldType::Asset; break;
//					}
//				}
//
//				switch (propType)
//				{
//					case Wire::ComponentRegistry::PropertyType::Bool: savedFieldMap.at(propName)->SetValue(*(bool*)&propData[0], sizeof(bool), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::String:
//					{
//						std::string str;
//						VT_DESERIALIZE_PROPERTY(data, str, m, std::string(""));
//
//						savedFieldMap.at(propName)->SetValue(*str.c_str(), sizeof(uint8_t) * str.size() + 1, fieldType);
//						break;
//					}
//
//					case Wire::ComponentRegistry::PropertyType::Int: savedFieldMap.at(propName)->SetValue(*(int32_t*)&propData[0], sizeof(int32_t), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::UInt: savedFieldMap.at(propName)->SetValue(*(uint32_t*)&propData[0], sizeof(uint32_t), fieldType); break;
//
//					case Wire::ComponentRegistry::PropertyType::Short: savedFieldMap.at(propName)->SetValue(*(int16_t*)&propData[0], sizeof(int16_t), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::UShort: savedFieldMap.at(propName)->SetValue(*(uint16_t*)&propData[0], sizeof(uint16_t), fieldType); break;
//
//					case Wire::ComponentRegistry::PropertyType::Char: savedFieldMap.at(propName)->SetValue(*(int8_t*)&propData[0], sizeof(int8_t), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::UChar: savedFieldMap.at(propName)->SetValue(*(uint8_t*)&propData[0], sizeof(uint8_t), fieldType); break;
//
//					case Wire::ComponentRegistry::PropertyType::Float: savedFieldMap.at(propName)->SetValue(*(float*)&propData[0], sizeof(float), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::Double: savedFieldMap.at(propName)->SetValue(*(double*)&propData[0], sizeof(double), fieldType); break;
//
//					case Wire::ComponentRegistry::PropertyType::Vector2: savedFieldMap.at(propName)->SetValue(*(glm::vec2*)&propData[0], sizeof(glm::vec2), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::Vector3: savedFieldMap.at(propName)->SetValue(*(glm::vec3*)&propData[0], sizeof(glm::vec3), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::Vector4: savedFieldMap.at(propName)->SetValue(*(glm::vec4*)&propData[0], sizeof(glm::vec4), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::Quaternion: savedFieldMap.at(propName)->SetValue(*(glm::vec4*)&propData[0], sizeof(glm::vec4), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::EntityId: savedFieldMap.at(propName)->SetValue(*(entt::entity*)&propData[0], sizeof(entt::entity), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::AssetHandle: savedFieldMap.at(propName)->SetValue(*(AssetHandle*)&propData[0], sizeof(AssetHandle), fieldType); break;
//
//					case Wire::ComponentRegistry::PropertyType::Color3: savedFieldMap.at(propName)->SetValue(*(glm::vec3*)&propData[0], sizeof(glm::vec3), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::Color4: savedFieldMap.at(propName)->SetValue(*(glm::vec4*)&propData[0], sizeof(glm::vec4), fieldType); break;
//					case Wire::ComponentRegistry::PropertyType::Enum: savedFieldMap.at(propName)->SetValue(*(uint32_t*)&propData[0], sizeof(uint32_t), fieldType); break;
//				}
//			}
//		}
//	}
//
//	void SceneImporter::SerializeEntityLayer(entt::entity id, const Wire::Registry& registry, YAML::Emitter& out, const AssetMetadata& metadata, const Ref<Scene>& scene) const
//	{
//		out << YAML::BeginMap;
//		out << YAML::Key << "Entity" << YAML::Value;
//		{
//			out << YAML::BeginMap;
//			VT_SERIALIZE_PROPERTY(id, id, out);
//
//			out << YAML::Key << "components" << YAML::BeginSeq;
//			for (const auto& [guid, pool] : registry.GetPools())
//			{
//				if (!pool->HasComponent(id))
//				{
//					continue;
//				}
//
//				auto* componentData = (uint8_t*)pool->GetComponent(id);
//				const auto& compInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);
//
//				out << YAML::BeginMap;
//				out << YAML::Key << "guid" << YAML::Value << guid;
//				out << YAML::Key << "properties" << YAML::BeginSeq;
//				for (const auto& prop : compInfo.properties)
//				{
//					if (prop.serializable)
//					{
//						if (registry.HasComponent<PrefabComponent>(id) && guid != PrefabComponent::comp_guid)
//						{
//							auto prefabComp = (PrefabComponent*)registry.GetComponentPtr(PrefabComponent::comp_guid, id);
//							if (prefabComp)
//							{
//								auto prefab = Volt::AssetManager::GetAsset<Prefab>(prefabComp->prefabAsset);
//
//								if (prefab && prefab->IsValid() && Prefab::IsValidInPrefab(prefabComp->prefabEntity, prefabComp->prefabAsset))
//								{
//									if (prefab->GetRegistry().HasComponent(guid, prefabComp->prefabEntity))
//									{
//										auto* prefabComponentData = (uint8_t*)prefab->GetRegistry().GetComponentPtr(guid, prefabComp->prefabEntity);
//
//										if (CheckComponentDataMatch(componentData, prefabComponentData, prop))
//										{
//											continue;
//										}
//									}
//								}
//							}
//						}
//
//						out << YAML::BeginMap;
//						VT_SERIALIZE_PROPERTY(type, prop.type, out);
//						VT_SERIALIZE_PROPERTY(vectorType, prop.vectorType, out);
//						VT_SERIALIZE_PROPERTY(name, prop.name, out);
//
//						myPropertySerializer[prop.type](componentData, prop.offset, prop.vectorType, out);
//
//						out << YAML::EndMap;
//					}
//				}
//				out << YAML::EndSeq;
//				out << YAML::EndMap;
//			}
//			out << YAML::EndSeq;
//
//			if (registry.HasComponent<VisualScriptingComponent>(id))
//			{
//				auto* vsComp = (VisualScriptingComponent*)registry.GetComponentPtr(VisualScriptingComponent::comp_guid, id);
//				if (vsComp->graph)
//				{
//					GraphKey::Graph::Serialize(vsComp->graph, out);
//				}
//			}
//
//			if (registry.HasComponent<VertexPaintedComponent>(id))
//			{
//				std::filesystem::path vpPath = (ProjectManager::GetDirectory() / metadata.filePath.parent_path() / "Layers" / ("ent_" + std::to_string(id) + ".entVp"));
//				auto* vpComp = (VertexPaintedComponent*)registry.GetComponentPtr(VertexPaintedComponent::comp_guid, id);
//
//				if (std::filesystem::exists(vpPath))
//				{
//					using std::filesystem::perms;
//					std::filesystem::permissions(vpPath, perms::_All_write);
//				}
//
//				BinarySerializer binaryVp(vpPath, sizeof(uint32_t) * vpComp->vertexColors.size() + sizeof(vpComp->meshHandle));
//
//				binaryVp.Serialize(vpComp->vertexColors.data(), sizeof(uint32_t) * vpComp->vertexColors.size());
//				binaryVp.Serialize(vpComp->meshHandle);
//				binaryVp.WriteToFile();
//			}
//
//			if (registry.HasComponent<MonoScriptComponent>(id))
//			{
//				SerializeMono(id, scene->GetScriptFieldCache(), registry, out);
//			}
//			out << YAML::EndMap;
//		}
//		out << YAML::EndMap;
//	}
//
//	void SceneImporter::DeserializeEntityLayer(Wire::Registry& registry, const AssetMetadata& metadata, Ref<Scene> scene, const YAML::Node& node) const
//	{
//		VT_PROFILE_FUNCTION();
//
//		YAML::Node entityNode = node["Entity"];
//
//		entt::entity entityId = Wire::NullID;
//		VT_DESERIALIZE_PROPERTY(id, entityId, entityNode, (entt::entity)Wire::NullID);
//
//		if (entityId == Wire::NullID)
//		{
//			return;
//		}
//
//		YAML::Node componentsNode = entityNode["components"];
//		for (auto compNode : componentsNode)
//		{
//			WireGUID componentGUID;
//			VT_DESERIALIZE_PROPERTY(guid, componentGUID, compNode, WireGUID::Null());
//			if (componentGUID == WireGUID::Null())
//			{
//				continue;
//			}
//
//			if (componentGUID == PrefabComponent::comp_guid)
//			{
//				auto* componentData = (registry.HasComponent(componentGUID, entityId)) ? (uint8_t*)registry.GetComponentPtr(componentGUID, entityId) : (uint8_t*)registry.AddComponent(componentGUID, entityId);
//
//				YAML::Node propertiesNode = compNode["properties"];
//				if (propertiesNode)
//				{
//					const auto& regInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(componentGUID);
//
//					for (auto propNode : propertiesNode)
//					{
//						Wire::ComponentRegistry::PropertyType type;
//						Wire::ComponentRegistry::PropertyType vectorType;
//						std::string name;
//
//						VT_DESERIALIZE_PROPERTY(type, type, propNode, Wire::ComponentRegistry::PropertyType::Unknown);
//						VT_DESERIALIZE_PROPERTY(vectorType, vectorType, propNode, Wire::ComponentRegistry::PropertyType::Unknown);
//						VT_DESERIALIZE_PROPERTY(name, name, propNode, std::string("Null"));
//
//						if (type == Wire::ComponentRegistry::PropertyType::Unknown)
//						{
//							continue;
//						}
//
//						// Try to find property
//						auto it = std::find_if(regInfo.properties.begin(), regInfo.properties.end(), [name, type](const auto& prop)
//						{
//							return prop.name == name && prop.type == type;
//						});
//
//						if (it != regInfo.properties.end())
//						{
//							myPropertyDeserializer[type](componentData, it->offset, it->vectorType, propNode);
//						}
//					}
//				}
//
//				break;
//			}
//		}
//
//		if (registry.HasComponent<PrefabComponent>(entityId))
//		{
//			auto prefabComp = (PrefabComponent*)registry.GetComponentPtr(PrefabComponent::comp_guid, entityId);
//			auto prefab = AssetManager::GetAsset<Prefab>(prefabComp->prefabAsset);
//			
//			if (prefab && prefab->IsValid() && Prefab::IsValidInPrefab(prefabComp->prefabEntity, prefabComp->prefabAsset))
//			{
//				entityId = registry.AddEntity(entityId);
//				Entity::Copy(prefab->GetRegistry(), registry, prefab->GetScriptFieldCache(), scene->GetScriptFieldCache(), prefabComp->prefabEntity, entityId);
//			}
//			else
//			{
//				return;
//			}
//		}
//		else
//		{
//			entityId = registry.AddEntity(entityId);
//		}
//
//		if (componentsNode)
//		{
//			VT_PROFILE_SCOPE("Components");
//
//			for (auto compNode : componentsNode)
//			{
//				WireGUID componentGUID;
//				VT_DESERIALIZE_PROPERTY(guid, componentGUID, compNode, WireGUID::Null());
//				if (componentGUID == WireGUID::Null())
//				{
//					continue;
//				}
//
//				auto* componentData = (registry.HasComponent(componentGUID, entityId)) ? (uint8_t*)registry.GetComponentPtr(componentGUID, entityId) : (uint8_t*)registry.AddComponent(componentGUID, entityId);
//
//				YAML::Node propertiesNode = compNode["properties"];
//				if (propertiesNode)
//				{
//					const auto& regInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(componentGUID);
//
//					for (auto propNode : propertiesNode)
//					{
//						Wire::ComponentRegistry::PropertyType type;
//						Wire::ComponentRegistry::PropertyType vectorType;
//						std::string name;
//
//						VT_DESERIALIZE_PROPERTY(type, type, propNode, Wire::ComponentRegistry::PropertyType::Unknown);
//						VT_DESERIALIZE_PROPERTY(vectorType, vectorType, propNode, Wire::ComponentRegistry::PropertyType::Unknown);
//						VT_DESERIALIZE_PROPERTY(name, name, propNode, std::string("Null"));
//
//						if (type == Wire::ComponentRegistry::PropertyType::Unknown)
//						{
//							continue;
//						}
//
//						// Try to find property
//						auto it = std::find_if(regInfo.properties.begin(), regInfo.properties.end(), [name, type](const auto& prop)
//						{
//							return prop.name == name && prop.type == type;
//						});
//
//
//						if (type == Wire::ComponentRegistry::PropertyType::Vector)
//						{
//							std::vector<uint32_t>& vec = *(std::vector<uint32_t>*)&componentData[it->offset];
//							vec.clear();
//						}
//
//						if (it != regInfo.properties.end())
//						{
//							myPropertyDeserializer[type](componentData, it->offset, it->vectorType, propNode);
//						}
//					}
//				}
//			}
//		}
//
//		YAML::Node graphNode = entityNode["Graph"];
//		if (graphNode && registry.HasComponent<VisualScriptingComponent>(entityId))
//		{
//			auto& vsComp = registry.GetComponent<VisualScriptingComponent>(entityId);
//			vsComp.graph = CreateRef<GraphKey::Graph>(entityId);
//
//			GraphKey::Graph::Deserialize(vsComp.graph, graphNode);
//		}
//
//		if (registry.HasComponent<VertexPaintedComponent>(entityId))
//		{
//			std::filesystem::path vpPath = metadata.filePath.parent_path();
//			vpPath = ProjectManager::GetDirectory() / vpPath / "Layers" / ("ent_" + std::to_string(entityId) + ".entVp");
//
//			if (std::filesystem::exists(vpPath))
//			{
//				auto* vpComp = (VertexPaintedComponent*)registry.GetComponentPtr(VertexPaintedComponent::comp_guid, entityId);
//				std::ifstream vpFile(vpPath, std::ios::in | std::ios::binary);
//				if (!vpFile.is_open())
//				{
//					VT_CORE_ERROR("Could not open entVp file!");
//				}
//
//				std::vector<uint8_t> totalData;
//				const size_t srcSize = vpFile.seekg(0, std::ios::end).tellg();
//				totalData.resize(srcSize);
//				vpFile.seekg(0, std::ios::beg);
//				vpFile.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
//				vpFile.close();
//
//				memcpy_s(&vpComp->meshHandle, sizeof(vpComp->meshHandle), totalData.data() + totalData.size() - sizeof(vpComp->meshHandle), sizeof(vpComp->meshHandle));
//				totalData.resize(totalData.size() - sizeof(vpComp->meshHandle));
//
//				vpComp->vertexColors.reserve(totalData.size() / sizeof(uint32_t));
//				for (size_t offset = 0; offset < totalData.size(); offset += sizeof(uint32_t))
//				{
//					uint32_t vpColor;
//					memcpy_s(&vpColor, sizeof(uint32_t), totalData.data() + offset, sizeof(uint32_t));
//					vpComp->vertexColors.push_back(vpColor);
//				}
//			}
//		}
//
//		YAML::Node monoNode = entityNode["MonoScripts"];
//		if (monoNode && registry.HasComponent<MonoScriptComponent>(entityId))
//		{
//			DeserializeMono(entityId, scene->GetScriptFieldCache(), entityNode);
//		}
//
//		if (registry.HasComponent<PrefabComponent>(entityId))
//		{
//			VT_PROFILE_SCOPE("Prefab Version Check");
//
//			auto& prefabComp = registry.GetComponent<PrefabComponent>(entityId);
//			const uint32_t prefabVersion = Prefab::GetPrefabVersion(prefabComp.prefabAsset);
//			const bool isParent = Prefab::IsRootInPrefab(prefabComp.prefabEntity, prefabComp.prefabAsset);
//
//			if (prefabComp.version < prefabVersion && isParent)
//			{
//				//prefabComp.isDirty = true;
//			}
//		}
//	}
//}

#include "Volt/Asset/AssetManager.h"

#include "Volt/Components/CoreComponents.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"
#include "Volt/Scene/Reflection/ComponentReflection.h"
#include "Volt/Scene/Reflection/ComponentRegistry.h"

namespace Volt
{
	template<typename T>
	void RegisterSerializationFunction(std::unordered_map<std::type_index, std::function<void(YAMLStreamWriter&, const uint8_t*, const size_t)>>& outTypes)
	{
		outTypes[std::type_index{ typeid(T) }] = [](YAMLStreamWriter& streamWriter, const uint8_t* data, const size_t offset) { streamWriter.SetKey("data", *reinterpret_cast<const T*>(&data[offset])); };
	}

	template<typename T>
	void RegisterDeserializationFunction(std::unordered_map<std::type_index, std::function<void(YAMLStreamReader&, uint8_t*, const size_t)>>& outTypes)
	{
		outTypes[std::type_index{ typeid(T) }] = [](YAMLStreamReader& streamReader, uint8_t* data, const size_t offset) { *reinterpret_cast<T*>(&data[offset]) = streamReader.ReadKey("data", T()); };
	}

	SceneImporter::SceneImporter()
	{
		RegisterSerializationFunction<int8_t>(s_typeSerializers);
		RegisterSerializationFunction<uint8_t>(s_typeSerializers);
		RegisterSerializationFunction<int16_t>(s_typeSerializers);
		RegisterSerializationFunction<uint16_t>(s_typeSerializers);
		RegisterSerializationFunction<int32_t>(s_typeSerializers);
		RegisterSerializationFunction<uint32_t>(s_typeSerializers);

		RegisterSerializationFunction<float>(s_typeSerializers);
		RegisterSerializationFunction<double>(s_typeSerializers);
		RegisterSerializationFunction<bool>(s_typeSerializers);

		RegisterSerializationFunction<glm::vec2>(s_typeSerializers);
		RegisterSerializationFunction<glm::vec3>(s_typeSerializers);
		RegisterSerializationFunction<glm::vec4>(s_typeSerializers);

		RegisterSerializationFunction<glm::uvec2>(s_typeSerializers);
		RegisterSerializationFunction<glm::uvec3>(s_typeSerializers);
		RegisterSerializationFunction<glm::uvec4>(s_typeSerializers);

		RegisterSerializationFunction<glm::ivec2>(s_typeSerializers);
		RegisterSerializationFunction<glm::ivec3>(s_typeSerializers);
		RegisterSerializationFunction<glm::ivec4>(s_typeSerializers);

		RegisterSerializationFunction<glm::quat>(s_typeSerializers);
		RegisterSerializationFunction<glm::mat4>(s_typeSerializers);
		RegisterSerializationFunction<VoltGUID>(s_typeSerializers);

		RegisterSerializationFunction<std::string>(s_typeSerializers);
		RegisterSerializationFunction<std::filesystem::path>(s_typeSerializers);

		RegisterSerializationFunction<entt::entity>(s_typeSerializers);
		RegisterSerializationFunction<AssetHandle>(s_typeSerializers);

		RegisterDeserializationFunction<int8_t>(s_typeDeserializers);	
		RegisterDeserializationFunction<uint8_t>(s_typeDeserializers);
		RegisterDeserializationFunction<int16_t>(s_typeDeserializers);
		RegisterDeserializationFunction<uint16_t>(s_typeDeserializers);
		RegisterDeserializationFunction<int32_t>(s_typeDeserializers);
		RegisterDeserializationFunction<uint32_t>(s_typeDeserializers);

		RegisterDeserializationFunction<float>(s_typeDeserializers);
		RegisterDeserializationFunction<double>(s_typeDeserializers);
		RegisterDeserializationFunction<bool>(s_typeDeserializers);
	
		RegisterDeserializationFunction<glm::vec2>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::vec3>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::vec4>(s_typeDeserializers);

		RegisterDeserializationFunction<glm::uvec2>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::uvec3>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::uvec4>(s_typeDeserializers);

		RegisterDeserializationFunction<glm::ivec2>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::ivec3>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::ivec4>(s_typeDeserializers);

		RegisterDeserializationFunction<glm::quat>(s_typeDeserializers);
		RegisterDeserializationFunction<glm::mat4>(s_typeDeserializers);
		RegisterDeserializationFunction<VoltGUID>(s_typeDeserializers);

		RegisterDeserializationFunction<std::string>(s_typeDeserializers);
		RegisterDeserializationFunction<std::filesystem::path>(s_typeDeserializers);

		RegisterDeserializationFunction<entt::entity>(s_typeDeserializers);
		RegisterDeserializationFunction<AssetHandle>(s_typeDeserializers);
	}

	bool SceneImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<Scene>();
		Ref<Scene> scene = reinterpret_pointer_cast<Scene>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLStreamReader streamReader{};

		if (!streamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("Failed to open file {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		streamReader.EnterScope("Scene");
		scene->myName = streamReader.ReadKey("name", std::string("New Scene"));
		streamReader.ExitScope();

		const std::filesystem::path& scenePath = filePath;
		std::filesystem::path folderPath = scenePath.parent_path();

		LoadSceneLayers(metadata, scene, folderPath);
		scene->SortScene();
		return true;
	}

	void SceneImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<Scene> scene = std::reinterpret_pointer_cast<Scene>(asset);

		std::filesystem::path folderPath = AssetManager::GetFilesystemPath(metadata.filePath);
		if (!std::filesystem::is_directory(folderPath))
		{
			folderPath = folderPath.parent_path();
		}

		std::filesystem::path scenePath = folderPath / (metadata.filePath.stem().string() + ".vtscene");

		YAMLStreamWriter streamWriter{ scenePath };
		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Scene");
		streamWriter.SetKey("name", metadata.filePath.stem().string());
		streamWriter.EndMap();
		streamWriter.EndMap();
		streamWriter.WriteToDisk();

		SaveSceneLayers(metadata, scene, folderPath);
	}

	void SceneImporter::LoadSceneLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
	{
		std::filesystem::path layersFolderPath = sceneDirectory / "Layers";
		if (!std::filesystem::exists(layersFolderPath))
		{
			return;
		}

		std::vector<std::filesystem::path> layerPaths;

		for (const auto& it : std::filesystem::directory_iterator(layersFolderPath))
		{
			if (!it.is_directory() && it.path().extension().string() == ".vtlayer")
			{
				layerPaths.emplace_back(it.path());
			}
		}

		std::vector<SceneLayer> sceneLayers;

		for (const auto& layerPath : layerPaths)
		{
			auto& sceneLayer = sceneLayers.emplace_back();

			YAMLStreamReader streamReader{};
			if (!streamReader.OpenFile(layerPath))
			{
				continue;
			}

			streamReader.EnterScope("Layer");
			{
				sceneLayer.name = streamReader.ReadKey("name", std::string("Null"));
				sceneLayer.id = streamReader.ReadKey("id", 0u);
				sceneLayer.visible = streamReader.ReadKey("visible", true);
				sceneLayer.locked = streamReader.ReadKey("locked", false);
			
				streamReader.ForEach("Entities", [&]() 
				{
					DeserializeEntity(scene, metadata, streamReader);
				});
			}
			streamReader.ExitScope();
		}
	}

	void SceneImporter::SaveSceneLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const
	{
		std::filesystem::path layerFolderPath = sceneDirectory / "Layers";
		if (!std::filesystem::exists(layerFolderPath))
		{
			std::filesystem::create_directories(layerFolderPath);
		}

		for (const auto& layer : scene->GetLayers())
		{
			const auto layerPath = layerFolderPath / ("layer_" + std::to_string(layer.id) + ".vtlayer");

			YAMLStreamWriter streamWriter{ layerPath };
			streamWriter.BeginMap();
			streamWriter.BeginMapNamned("Layer");
			{
				streamWriter.SetKey("name", layer.name);
				streamWriter.SetKey("id", layer.id);
				streamWriter.SetKey("visible", layer.visible);
				streamWriter.SetKey("locked", layer.locked);

				streamWriter.BeginSequence("Entities");
				{
					for (const auto id : scene->GetAllEntities())
					{
						Volt::Entity entity{ id, scene };
						const uint32_t layerId = entity.GetComponent<CommonComponent>().layerId;

						if (layerId != layer.id)
						{
							continue;
						}

						SerializeEntity(id, scene, streamWriter);
					}
				}
				streamWriter.EndSequence();
			}
			streamWriter.EndMap();
			streamWriter.EndMap();
			streamWriter.WriteToDisk();
		}
	}

	void SceneImporter::SerializeEntity(entt::entity id, const Ref<Scene>& scene, YAMLStreamWriter& streamWriter) const
	{
		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Entity");

		auto& registry = scene->GetRegistry();

		streamWriter.SetKey("id", id);
		streamWriter.BeginSequence("components");
		{
			for (auto&& curr : registry.storage())
			{
				auto& storage = curr.second;

				if (!storage.contains(id))
				{
					continue;
				}

				const IComponentTypeDesc* componentDesc = reinterpret_cast<const IComponentTypeDesc*>(Volt::ComponentRegistry::GetTypeDescFromName(storage.type().name()));
				const uint8_t* componentPtr = reinterpret_cast<const uint8_t*>(storage.get(id));

				SerializeClass(componentPtr, 0, componentDesc, streamWriter);
			}
		}
		streamWriter.EndSequence();

		streamWriter.EndMap();
		streamWriter.EndMap();
	}

	void SceneImporter::SerializeClass(const uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, YAMLStreamWriter& streamWriter) const
	{
		streamWriter.BeginMap();
		streamWriter.SetKey("guid", compDesc->GetGUID());
		streamWriter.BeginSequence("members");

		for (const auto& member : compDesc->GetMembers())
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("name", member.name);

			if (member.typeDesc != nullptr)
			{
				switch (member.typeDesc->GetValueType())
				{
					case ValueType::Component:
						streamWriter.SetKey("data", "component");
						SerializeClass(data, offset + member.offset, compDesc, streamWriter);
						break;

					case ValueType::Enum:
						streamWriter.SetKey("data", "enum");
						streamWriter.SetKey("enumValue", *reinterpret_cast<const int32_t*>(&data[member.offset]));
						break;
				}
			}
			else
			{
				if (s_typeSerializers.contains(member.typeIndex))
				{
					s_typeSerializers.at(member.typeIndex)(streamWriter, data, member.offset);
				}
			}

			streamWriter.EndMap();
		}

		streamWriter.EndSequence();
		streamWriter.EndMap();
	}

	void SceneImporter::DeserializeEntity(const Ref<Scene>& scene, const AssetMetadata& metadata, YAMLStreamReader& streamReader) const
	{
		streamReader.EnterScope("Entity");

		entt::entity entityId = streamReader.ReadKey("id", (entt::entity)entt::null);
		 
		if (entityId == entt::null)
		{
			return;
		}

		entityId = scene->GetRegistry().create(entityId);

		streamReader.ForEach("components", [&]() 
		{
			VoltGUID compGuid = streamReader.ReadKey("guid", VoltGUID::Null());
			if (compGuid == VoltGUID::Null())
			{
				return;
			}

			const ICommonTypeDesc* typeDesc = Volt::ComponentRegistry::GetTypeDescFromGUID(compGuid);
			if (!typeDesc)
			{
				return;
			}

			switch (typeDesc->GetValueType())
			{
				case ValueType::Component:
				{
					Volt::ComponentRegistry::Helpers::AddComponentWithGUID(compGuid, scene->GetRegistry(), entityId);
					void* voidCompPtr = Volt::ComponentRegistry::Helpers::GetComponentWithGUID(compGuid, scene->GetRegistry(), entityId);
					uint8_t* componentData = reinterpret_cast<uint8_t*>(voidCompPtr);

					const IComponentTypeDesc* componentDesc = reinterpret_cast<const IComponentTypeDesc*>(typeDesc);
					DeserializeClass(componentData, 0, componentDesc, streamReader);
					break;
				}
			}
			
		});

		streamReader.ExitScope();
	}

	void SceneImporter::DeserializeClass(uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, YAMLStreamReader& streamReader) const
	{
		streamReader.ForEach("members", [&]() 
		{
			const std::string memberName = streamReader.ReadKey("name", std::string(""));
			if (memberName.empty())
			{
				return;
			}

			const ComponentMember* componentMember = const_cast<IComponentTypeDesc*>(compDesc)->FindMemberByName(memberName);
		
			if (componentMember->typeDesc != nullptr)
			{
				switch (componentMember->typeDesc->GetValueType())
				{
					case ValueType::Component:
					{
						const IComponentTypeDesc* memberCompType = reinterpret_cast<const IComponentTypeDesc*>(componentMember->typeDesc);
						DeserializeClass(data, offset + componentMember->offset, memberCompType, streamReader);
						break;
					}

					case ValueType::Enum:
					{
						*reinterpret_cast<int32_t*>(&data[offset + componentMember->offset]) = streamReader.ReadKey("enumValue", int32_t(0));
						break;
					}
				}
			}
			else
			{
				if (s_typeDeserializers.contains(componentMember->typeIndex))
				{
					s_typeDeserializers.at(componentMember->typeIndex)(streamReader, data, offset + componentMember->offset);
				}
			}
		});
	}
}
