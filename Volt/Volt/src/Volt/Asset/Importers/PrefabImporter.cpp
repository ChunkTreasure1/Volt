#include "vtpch.h"
#include "PrefabImporter.h"

#include "Volt/Asset/Prefab.h"
#include "Volt/Log/Log.h"
#include "Volt/Components/Components.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/SerializationMacros.h"

#include <Wire/Wire.h>
#include <yaml-cpp/yaml.h>

namespace Volt
{
	template<typename T>
	inline static void DeserializeVector(uint8_t* data, uint32_t offset, YAML::Node node, T defaultValue)
	{
		std::vector<T>& vector = *(std::vector<T>*) & data[offset];
		for (const auto& vecNode : node)
		{
			VT_DESERIALIZE_PROPERTY(value, vector.emplace_back(), vecNode, defaultValue);
		}
	}

	template<typename T>
	inline static void SerializeVector(uint8_t* data, uint32_t offset, YAML::Emitter& out)
	{
		std::vector<T>& items = *(std::vector<T>*) & data[offset];
		for (const auto& item : items)
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(value, item, out);
			out << YAML::EndMap;
		}
	}

	bool PrefabImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Prefab>();
		Wire::Registry registry;

		if (!std::filesystem::exists(path)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(path);
		if (!file.is_open()) [[unlikely]]
		{
			VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", path, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		uint32_t version = 0;
		AssetHandle assetHandle;

		YAML::Node prefabNode = root["Prefab"];
		VT_DESERIALIZE_PROPERTY(assetHandle, assetHandle, prefabNode, AssetHandle(0));
		VT_DESERIALIZE_PROPERTY(version, version, prefabNode, 0);

		YAML::Node entitiesNode = prefabNode["entities"];

		for (const auto& entityNode : entitiesNode)
		{
			Wire::EntityId entityId = Wire::NullID;
			VT_DESERIALIZE_PROPERTY(id, entityId, entityNode, (Wire::EntityId)Wire::NullID);

			if (entityId == Wire::NullID)
			{
				continue;
			}

			registry.AddEntity(entityId);

			YAML::Node componentsNode = entityNode["components"];
			if (componentsNode)
			{
				for (auto compNode : componentsNode)
				{
					WireGUID componentGUID;
					VT_DESERIALIZE_PROPERTY(guid, componentGUID, compNode, WireGUID::Null());
					if (componentGUID == WireGUID::Null())
					{
						continue;
					}

					auto* componentData = (uint8_t*)registry.AddComponent(componentGUID, entityId);

					YAML::Node propertiesNode = compNode["properties"];
					if (propertiesNode)
					{
						auto regInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(componentGUID);

						for (auto propNode : propertiesNode)
						{
							Wire::ComponentRegistry::PropertyType type;
							Wire::ComponentRegistry::PropertyType vectorType;
							std::string name;

							VT_DESERIALIZE_PROPERTY(type, type, propNode, Wire::ComponentRegistry::PropertyType::Unknown);
							VT_DESERIALIZE_PROPERTY(vectorType, vectorType, propNode, Wire::ComponentRegistry::PropertyType::Unknown);
							VT_DESERIALIZE_PROPERTY(name, name, propNode, std::string("Null"));

							if (type == Wire::ComponentRegistry::PropertyType::Unknown)
							{
								continue;
							}

							// Try to find property
							auto it = std::find_if(regInfo.properties.begin(), regInfo.properties.end(), [name, type](const auto& prop)
								{
									return prop.name == name && prop.type == type;
								});

							if (it != regInfo.properties.end())
							{
								switch (type)
								{
									case Wire::ComponentRegistry::PropertyType::Bool:
									{
										bool input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, false);
										memcpy_s(&componentData[it->offset], sizeof(bool), &input, sizeof(bool));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Int:
									{
										int32_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, 0);
										memcpy_s(&componentData[it->offset], sizeof(int32_t), &input, sizeof(int32_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::UInt:
									{
										uint32_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, 0);
										memcpy_s(&componentData[it->offset], sizeof(uint32_t), &input, sizeof(uint32_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Short:
									{
										int16_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, int16_t(0));
										memcpy_s(&componentData[it->offset], sizeof(int16_t), &input, sizeof(int16_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::UShort:
									{
										uint16_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, uint16_t(0));
										memcpy_s(&componentData[it->offset], sizeof(uint16_t), &input, sizeof(uint16_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Char:
									{
										int8_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, int8_t(0));
										memcpy_s(&componentData[it->offset], sizeof(int8_t), &input, sizeof(int8_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::UChar:
									{
										uint8_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, uint8_t(0));
										memcpy_s(&componentData[it->offset], sizeof(uint8_t), &input, sizeof(uint8_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Float:
									{
										float input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, 0.f);
										memcpy_s(&componentData[it->offset], sizeof(float), &input, sizeof(float));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Double:
									{
										double input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, 0.0);
										memcpy_s(&componentData[it->offset], sizeof(double), &input, sizeof(double));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Vector2:
									{
										gem::vec2 input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, gem::vec2{ 0.f });
										memcpy_s(&componentData[it->offset], sizeof(gem::vec2), &input, sizeof(gem::vec2));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Vector3:
									{
										gem::vec3 input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, gem::vec3(0.f));
										memcpy_s(&componentData[it->offset], sizeof(gem::vec3), &input, sizeof(gem::vec3));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Vector4:
									{
										gem::vec4 input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, gem::vec4(0.f));
										memcpy_s(&componentData[it->offset], sizeof(gem::vec4), &input, sizeof(gem::vec4));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::String:
									{
										std::string input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, std::string(""));

										(*(std::string*)&componentData[it->offset]) = input;

										break;
									}

									case Wire::ComponentRegistry::PropertyType::Int64:
									{
										int64_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, 0);
										memcpy_s(&componentData[it->offset], sizeof(int64_t), &input, sizeof(int64_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::UInt64:
									{
										uint64_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, 0);
										memcpy_s(&componentData[it->offset], sizeof(uint64_t), &input, sizeof(uint64_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::AssetHandle:
									{
										AssetHandle input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, AssetHandle(0));
										memcpy_s(&componentData[it->offset], sizeof(AssetHandle), &input, sizeof(AssetHandle));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Color3:
									{
										gem::vec3 input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, gem::vec3(0.f));
										memcpy_s(&componentData[it->offset], sizeof(gem::vec3), &input, sizeof(gem::vec3));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Color4:
									{
										gem::vec4 input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, gem::vec4(0.f));
										memcpy_s(&componentData[it->offset], sizeof(gem::vec4), &input, sizeof(gem::vec4));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Folder:
									{
										std::filesystem::path input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, std::filesystem::path(""));

										(*(std::filesystem::path*)&componentData[it->offset]) = input;
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Path:
									{
										std::filesystem::path input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, std::filesystem::path(""));

										(*(std::filesystem::path*)&componentData[it->offset]) = input;
										break;
									}

									case Wire::ComponentRegistry::PropertyType::EntityId:
									{
										uint32_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, 0);
										memcpy_s(&componentData[it->offset], sizeof(uint32_t), &input, sizeof(uint32_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::GUID:
									{
										WireGUID input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, WireGUID::Null());
										memcpy_s(&componentData[it->offset], sizeof(WireGUID), &input, sizeof(WireGUID));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Enum:
									{
										uint32_t input;
										VT_DESERIALIZE_PROPERTY(data, input, propNode, 0);
										memcpy_s(&componentData[it->offset], sizeof(uint32_t), &input, sizeof(uint32_t));
										break;
									}

									case Wire::ComponentRegistry::PropertyType::Vector:
									{
										if (!propNode["data"])
										{
											break;
										}

										switch (vectorType)
										{
											case Wire::ComponentRegistry::PropertyType::Bool: DeserializeVector<bool>(componentData, it->offset, propNode["data"], false); break;
											case Wire::ComponentRegistry::PropertyType::Int: DeserializeVector<int32_t>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::UInt: DeserializeVector<uint32_t>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Short: DeserializeVector<int16_t>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::UShort: DeserializeVector<uint16_t>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Char: DeserializeVector<int8_t>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::UChar: DeserializeVector<uint8_t>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Float: DeserializeVector<float>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Double: DeserializeVector<double>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Vector2: DeserializeVector<gem::vec2>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Vector3: DeserializeVector<gem::vec3>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Vector4: DeserializeVector<gem::vec4>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::String: DeserializeVector<std::string>(componentData, it->offset, propNode["data"], "Null"); break;
											case Wire::ComponentRegistry::PropertyType::Int64: DeserializeVector<int64_t>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::UInt64: DeserializeVector<uint64_t>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::AssetHandle: DeserializeVector<AssetHandle>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Color3: DeserializeVector<gem::vec3>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Color4: DeserializeVector<gem::vec4>(componentData, it->offset, propNode["data"], 0); break;
											case Wire::ComponentRegistry::PropertyType::Folder: DeserializeVector<std::filesystem::path>(componentData, it->offset, propNode["data"], "Null"); break;
											case Wire::ComponentRegistry::PropertyType::Path: DeserializeVector<std::filesystem::path>(componentData, it->offset, propNode["data"], "Null"); break;
											case Wire::ComponentRegistry::PropertyType::GUID: DeserializeVector<WireGUID>(componentData, it->offset, propNode["data"], WireGUID::Null()); break;
											case Wire::ComponentRegistry::PropertyType::EntityId: DeserializeVector<Wire::EntityId>(componentData, it->offset, propNode["data"], Wire::NullID); break;
											case Wire::ComponentRegistry::PropertyType::Enum: DeserializeVector<uint32_t>(componentData, it->offset, propNode["data"], 0); break;

										}

										break;
									}
								}
							}
						}
					}
				}
			}

			if (!registry.HasComponent<EntityDataComponent>(entityId))
			{
				registry.AddComponent<EntityDataComponent>(entityId);
			}
		}

		Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(asset);
		prefab->myRegistry = registry;
		prefab->myVersion = version;
		prefab->path = path;

		if (prefab->handle != assetHandle && assetHandle != Asset::Null())
		{
			VT_CORE_ERROR("Asset handle mismatch in prefab {0}! Please correct the asset registry! Registry: {1}, Asset: {2}", path.string(), prefab->handle, assetHandle);
		}

		return true;
	}

	void PrefabImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<Prefab> prefab = std::reinterpret_pointer_cast<Prefab>(asset);
		auto& registry = prefab->myRegistry;

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Prefab" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(version, prefab->myVersion, out);
			VT_SERIALIZE_PROPERTY(handle, prefab->handle, out);

			out << YAML::Key << "entities" << YAML::BeginSeq;
			for (const auto& id : registry.GetAllEntities())
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(id, id, out);
				{

					out << YAML::Key << "components" << YAML::BeginSeq;
					for (const auto& [guid, pool] : registry.GetPools())
					{
						if (!pool->HasComponent(id))
						{
							continue;
						}

						auto* componentData = (uint8_t*)pool->GetComponent(id);
						const auto& compInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);

						out << YAML::BeginMap;
						out << YAML::Key << "guid" << YAML::Value << guid;
						out << YAML::Key << "properties" << YAML::BeginSeq;
						for (const auto& prop : compInfo.properties)
						{
							if (prop.serializable)
							{
								out << YAML::BeginMap;
								VT_SERIALIZE_PROPERTY(type, prop.type, out);
								VT_SERIALIZE_PROPERTY(vectorType, prop.vectorType, out);
								VT_SERIALIZE_PROPERTY(name, prop.name, out);
								switch (prop.type)
								{
									case Wire::ComponentRegistry::PropertyType::Bool: VT_SERIALIZE_PROPERTY(data, *(bool*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Int: VT_SERIALIZE_PROPERTY(data, *(int32_t*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::UInt: VT_SERIALIZE_PROPERTY(data, *(uint32_t*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Short: VT_SERIALIZE_PROPERTY(data, *(int16_t*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::UShort: VT_SERIALIZE_PROPERTY(data, *(uint32_t*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Char: VT_SERIALIZE_PROPERTY(data, *(int8_t*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::UChar: VT_SERIALIZE_PROPERTY(data, *(uint8_t*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Float: VT_SERIALIZE_PROPERTY(data, *(float*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Double: VT_SERIALIZE_PROPERTY(data, *(double*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Vector2: VT_SERIALIZE_PROPERTY(data, *(gem::vec2*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Vector3: VT_SERIALIZE_PROPERTY(data, *(gem::vec3*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Vector4: VT_SERIALIZE_PROPERTY(data, *(gem::vec4*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::String:
									{
										std::string str = *(std::string*)&componentData[prop.offset];
										VT_SERIALIZE_PROPERTY(data, str, out);
										break;
									}
									case Wire::ComponentRegistry::PropertyType::Int64: VT_SERIALIZE_PROPERTY(data, *(int64_t*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::UInt64: VT_SERIALIZE_PROPERTY(data, *(uint64_t*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::AssetHandle: VT_SERIALIZE_PROPERTY(data, *(AssetHandle*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Color3: VT_SERIALIZE_PROPERTY(data, *(gem::vec3*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Color4: VT_SERIALIZE_PROPERTY(data, *(gem::vec4*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Folder: VT_SERIALIZE_PROPERTY(data, *(std::filesystem::path*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Path: VT_SERIALIZE_PROPERTY(data, *(std::filesystem::path*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::GUID: VT_SERIALIZE_PROPERTY(data, *(WireGUID*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::EntityId: VT_SERIALIZE_PROPERTY(data, *(uint32_t*)&componentData[prop.offset], out); break;
									case Wire::ComponentRegistry::PropertyType::Enum: VT_SERIALIZE_PROPERTY(data, *(uint32_t*)&componentData[prop.offset], out); break;

									case Wire::ComponentRegistry::PropertyType::Vector:
									{
										out << YAML::Key << "data" << YAML::BeginSeq;
										switch (prop.vectorType)
										{
											case Wire::ComponentRegistry::PropertyType::Bool: SerializeVector<bool>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Int: SerializeVector<int32_t>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::UInt: SerializeVector<uint32_t>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Short: SerializeVector<int16_t>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::UShort: SerializeVector<uint16_t>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Char: SerializeVector<int8_t>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::UChar: SerializeVector<uint8_t>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Float: SerializeVector<float>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Double: SerializeVector<double>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Vector2: SerializeVector<gem::vec2>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Vector3: SerializeVector<gem::vec3>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Vector4: SerializeVector<gem::vec4>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::String: SerializeVector<std::string>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Int64: SerializeVector<int64_t>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::UInt64: SerializeVector<uint64_t>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::AssetHandle: SerializeVector<AssetHandle>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Color3: SerializeVector<gem::vec3>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Color4: SerializeVector<gem::vec4>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Folder: SerializeVector<std::filesystem::path>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Path: SerializeVector<std::filesystem::path>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::GUID: SerializeVector<WireGUID>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::EntityId: SerializeVector<Wire::EntityId>(componentData, prop.offset, out); break;
											case Wire::ComponentRegistry::PropertyType::Enum: SerializeVector<uint32_t>(componentData, prop.offset, out); break;
										}

										out << YAML::EndSeq;
										break;
									}
								}
								out << YAML::EndMap;
							}
						}
						out << YAML::EndSeq;
						out << YAML::EndMap;
					}
					out << YAML::EndSeq;
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(asset->path);
		fout << out.c_str();
		fout.close();
	}
	void PrefabImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{}
	bool PrefabImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}
}