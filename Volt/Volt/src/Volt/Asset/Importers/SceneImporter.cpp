#include "vtpch.h"
#include "SceneImporter.h"

#include "Volt/Scene/Scene.h"

#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include "Volt/Components/Components.h"
#include "Volt/Asset/Prefab.h"

#include "Volt/Core/Profiling.h"
#include "Volt/Utility/FileSystem.h"

#include "Volt/Scene/Entity.h"

#include <GraphKey/Graph.h>
#include <GraphKey/Node.h>
#include <GraphKey/Registry.h>

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

	bool SceneImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		VT_PROFILE_FUNCTION();

		asset = CreateRef<Scene>();
		Ref<Scene> scene = reinterpret_pointer_cast<Scene>(asset);

		if (!std::filesystem::exists(path)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(path);
		if (!file.is_open()) [[unlikely]]
		{
			VT_CORE_ERROR("Failed to open file {0}!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		const std::filesystem::path& scenePath = path;
		std::filesystem::path folderPath = scenePath.parent_path();
		std::filesystem::path entitiesFolderPath = folderPath / "Entities";

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
			VT_CORE_ERROR("{0} contains invalid YAML with error {1}! Please correct it!", path, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node sceneNode = root["Scene"];

		VT_DESERIALIZE_PROPERTY(name, scene->myName, sceneNode, std::string("New Scene"));

		if (std::filesystem::exists(entitiesFolderPath))
		{
			std::vector<std::filesystem::path> entityPaths;

			for (const auto& it : std::filesystem::directory_iterator(entitiesFolderPath))
			{
				if (!it.is_directory())
				{
					entityPaths.emplace_back(it.path());
				}
			}

			// Divide paths into threads and load
			{
				const uint32_t threadCount = std::thread::hardware_concurrency();
				std::vector<std::vector<std::filesystem::path>> threadPathLists(gem::min(entityPaths.size(), (size_t)threadCount));

				const size_t perThreadEntityCount = entityPaths.size() / threadPathLists.size();
				for (size_t index = 0, offset = 0; auto & pathList : threadPathLists)
				{
					index++;
					if (index == threadPathLists.size())
					{
						pathList = { entityPaths.begin() + offset, entityPaths.end() };
					}
					else
					{
						pathList = { entityPaths.begin() + offset, entityPaths.begin() + offset + perThreadEntityCount };
					}

					offset += perThreadEntityCount;
				}

				auto threadFunc = [&](const std::string& name, const std::vector<std::filesystem::path>& paths, Wire::Registry& registry)
				{
					VT_PROFILE_THREAD(name.c_str());
					for (const auto& item : paths)
					{
						DeserializeEntity(item, registry, scene);
					}
				};

				std::vector<Wire::Registry> threadRegistries{ threadPathLists.size() };
				std::vector<std::thread> threads;

				for (uint32_t i = 0; i < (uint32_t)threadPathLists.size(); i++)
				{
					const std::string name = "Import Worker#" + std::to_string(i);
					threads.emplace_back(threadFunc, name, std::reference_wrapper(threadPathLists.at(i)), std::reference_wrapper(threadRegistries.at(i)));
				}

				for (auto& thread : threads)
				{
					thread.join();
				}

				for (auto& registry : threadRegistries)
				{
					for (const auto& entity : registry.GetAllEntities())
					{
						scene->myRegistry.AddEntity(entity);
						Entity::Copy(registry, scene->myRegistry, entity, entity);
					}
				}
			}

			scene->myRegistry.ForEach<PrefabComponent>([&](Wire::EntityId id, PrefabComponent& prefabComp)
				{
					if (prefabComp.isDirty)
					{
						Prefab::OverridePrefabInRegistry(scene->myRegistry, id, prefabComp.prefabAsset);
					}
				});
		}

		return true;
	}

	void SceneImporter::Save(const Ref<Asset>& asset) const
	{
		VT_PROFILE_FUNCTION();
		const Ref<Scene> scene = std::reinterpret_pointer_cast<Scene>(asset);

		std::filesystem::path folderPath = asset->path;
		if (!std::filesystem::is_directory(folderPath))
		{
			folderPath = folderPath.parent_path();
		}

		std::filesystem::path scenePath = folderPath / (asset->path.stem().string() + ".vtscene");
		std::filesystem::path entitiesFolderPath = folderPath / "Entities";

		if (!std::filesystem::exists(folderPath))
		{
			std::filesystem::create_directories(folderPath);
		}

		if (!std::filesystem::exists(entitiesFolderPath))
		{
			std::filesystem::create_directories(entitiesFolderPath);
		}

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(name, asset->path.stem(), out);
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream file;
		file.open(scenePath);
		file << out.c_str();
		file.close();

		/////Entities//////
		for (const auto entity : scene->GetRegistry().GetAllEntities())
		{
			SerializeEntity(entity, scene->GetRegistry(), entitiesFolderPath);
		}

		for (const auto& fileDir : std::filesystem::directory_iterator(entitiesFolderPath))
		{
			if (!fileDir.is_directory())
			{
				const auto stem = fileDir.path().stem().string();
				const size_t underscoreOffset = stem.find('_');

				if (underscoreOffset != std::string::npos)
				{
					const std::string entIdString = stem.substr(underscoreOffset + 1);
					int32_t id = std::stoi(entIdString); // Will break if id > 1.2 billion
					if (!scene->GetRegistry().Exists((uint32_t)id))
					{
						FileSystem::Remove(fileDir.path());
					}
				}
			}

		}

		asset->path = scenePath;
	}

	void SceneImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{}

	bool SceneImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}

	void SceneImporter::SerializeGraph(Ref<GraphKey::Graph> graph, const std::string& graphState, const Wire::Registry& registry, YAML::Emitter& out) const
	{
		out << YAML::Key << "Graph" << YAML::Value;
		{
			out << YAML::BeginMap;
			{
				out << YAML::Key << "Nodes" << YAML::BeginSeq;
				for (const auto& n : graph->GetSpecification().nodes)
				{
					out << YAML::BeginMap;
					{
						VT_SERIALIZE_PROPERTY(id, n->id, out);
						VT_SERIALIZE_PROPERTY(type, n->GetRegistryName(), out);

						out << YAML::Key << "inputs" << YAML::BeginSeq;
						{
							for (const auto& i : n->inputs)
							{
								out << YAML::BeginMap;
								{
									VT_SERIALIZE_PROPERTY(name, i.name, out);
									VT_SERIALIZE_PROPERTY(id, i.id, out);
								}
								out << YAML::EndMap;
							}
						}
						out << YAML::EndSeq;

						out << YAML::Key << "outputs" << YAML::BeginSeq;
						{
							for (const auto& o : n->outputs)
							{
								out << YAML::BeginMap;
								{
									VT_SERIALIZE_PROPERTY(name, o.name, out);
									VT_SERIALIZE_PROPERTY(id, o.id, out);
								}
								out << YAML::EndMap;
							}
							out << YAML::EndSeq;
						}
					}
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::Key << "Links" << YAML::BeginSeq;
				for (const auto& l : graph->GetSpecification().links)
				{
					out << YAML::BeginMap;
					{
						VT_SERIALIZE_PROPERTY(id, l->id, out);
						VT_SERIALIZE_PROPERTY(output, l->output, out);
						VT_SERIALIZE_PROPERTY(input, l->input, out);
					}
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}
			out << YAML::EndMap;
		}
	}

	void SceneImporter::DeserializeGraph(Ref<GraphKey::Graph> graph, const YAML::Node& node) const
	{
		struct Attribute
		{
			std::string name;
			UUID id;
		};

		struct NodeData
		{
			UUID id;
			std::string type;
			
			std::vector<Attribute> inputs;
			std::vector<Attribute> outputs;
		};

		struct LinkData
		{
			UUID id;
			
			UUID input;
			UUID output;
		};

		std::vector<NodeData> nodes;
		std::vector<LinkData> links;

		for (const auto& n : node["Nodes"])
		{
			auto& data = nodes.emplace_back();
			VT_DESERIALIZE_PROPERTY(id, data.id, n, UUID(0));
			VT_DESERIALIZE_PROPERTY(type, data.type, n, std::string(""));
			
			for (const auto& i : n["inputs"])
			{
				auto& inData = data.inputs.emplace_back();
				VT_DESERIALIZE_PROPERTY(id, inData.id, i, UUID(0));
				VT_DESERIALIZE_PROPERTY(name, inData.name, i, std::string("Null"));
			}
			
			for (const auto& o : n["outputs"])
			{
				auto& outData = data.outputs.emplace_back();
				VT_DESERIALIZE_PROPERTY(id, outData.id, o, UUID(0));
				VT_DESERIALIZE_PROPERTY(name, outData.name, o, std::string("Null"));
			}
		}

		for (const auto& l : node["Links"])
		{
			auto& data = links.emplace_back();
			VT_DESERIALIZE_PROPERTY(id, data.id, l, UUID(0));
			VT_DESERIALIZE_PROPERTY(input, data.input, l, UUID(0));
			VT_DESERIALIZE_PROPERTY(output, data.output, l, UUID(0));
		}

		auto& spec = graph->GetSpecification();

		for (const auto& n : nodes)
		{
			auto node = GraphKey::Registry::Create(n.type);
			node->id = n.id;
		
			for (const auto& i : n.inputs)
			{
				auto it = std::find_if(node->inputs.begin(), node->inputs.end(), [&i](const auto& lhs) 
					{
						return lhs.name == i.name;
					});

				if (it != node->inputs.end())
				{
					it->id = i.id;
				}
			}

			for (const auto& o : n.outputs)
			{
				auto it = std::find_if(node->outputs.begin(), node->outputs.end(), [&o](const auto& lhs)
					{
						return lhs.name == o.name;
					});

				if (it != node->outputs.end())
				{
					it->id = o.id;
				}
			}
		
			spec.nodes.emplace_back(node);
		}

		for (const auto& l : links)
		{
			Ref<GraphKey::Link> newLink = CreateRef<GraphKey::Link>();
			newLink->id = l.id;
			newLink->input = l.input;
			newLink->output = l.output;

			auto inAttr = graph->GetAttributeByID(l.input);
			inAttr->links.emplace_back(l.id);

			auto outAttr = graph->GetAttributeByID(l.output);
			outAttr->links.emplace_back(l.id);

			spec.links.emplace_back(newLink);
		}
	}

	void SceneImporter::SerializeEntity(Wire::EntityId id, const Wire::Registry& registry, const std::filesystem::path& targetDir) const
	{
		VT_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(id, id, out);

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
							case Wire::ComponentRegistry::PropertyType::Quaternion: VT_SERIALIZE_PROPERTY(data, *(gem::quat*)&componentData[prop.offset], out); break;
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
									case Wire::ComponentRegistry::PropertyType::Quaternion: SerializeVector<gem::quat>(componentData, prop.offset, out); break;
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

			if (registry.HasComponent<VisualScriptingComponent>(id))
			{
				auto* vsComp = (VisualScriptingComponent*)registry.GetComponentPtr(VisualScriptingComponent::comp_guid, id);
				if (vsComp->graph)
				{
					SerializeGraph(vsComp->graph, vsComp->graphState, registry, out);
				}
			}
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream output(targetDir / ("ent_" + std::to_string(id) + ".ent"), std::ios::out);
		output << out.c_str();
		output.close();
	}

	void SceneImporter::DeserializeEntity(const std::filesystem::path& path, Wire::Registry& registry, Ref<Scene> scene) const
	{
		VT_PROFILE_FUNCTION();

		std::ifstream file(path);
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
			return;
		}

		YAML::Node entityNode = root["Entity"];

		Wire::EntityId entityId = Wire::NullID;
		VT_DESERIALIZE_PROPERTY(id, entityId, entityNode, (Wire::EntityId)Wire::NullID);

		if (entityId == Wire::NullID)
		{
			return;
		}

		registry.AddEntity(entityId);

		YAML::Node componentsNode = entityNode["components"];
		if (componentsNode)
		{
			VT_PROFILE_SCOPE("Components");

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
					const auto& regInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(componentGUID);

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
									VT_DESERIALIZE_PROPERTY(data, input, propNode, 0u);
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

								case Wire::ComponentRegistry::PropertyType::Quaternion:
								{
									gem::quat input;
									VT_DESERIALIZE_PROPERTY(data, input, propNode, gem::quat(1.f, 0.f, 0.f, 0.f));
									memcpy_s(&componentData[it->offset], sizeof(gem::quat), &input, sizeof(gem::quat));
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
										case Wire::ComponentRegistry::PropertyType::Quaternion: DeserializeVector<gem::quat>(componentData, it->offset, propNode["data"], gem::quat{ 1.f, 0.f, 0.f, 0.f }); break;
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

		YAML::Node graphNode = entityNode["Graph"];
		if (graphNode && registry.HasComponent<VisualScriptingComponent>(entityId))
		{
			auto& vsComp = registry.GetComponent<VisualScriptingComponent>(entityId);
			vsComp.graph = CreateRef<GraphKey::Graph>(entityId);

			DeserializeGraph(vsComp.graph, graphNode);
		}

		if (registry.HasComponent<PrefabComponent>(entityId))
		{
			VT_PROFILE_SCOPE("Prefab Version Check")

				auto& prefabComp = registry.GetComponent<PrefabComponent>(entityId);
			const uint32_t prefabVersion = Prefab::GetPrefabVersion(prefabComp.prefabAsset);
			const bool isParent = Prefab::IsParentInPrefab(prefabComp.prefabEntity, prefabComp.prefabAsset);

			if (prefabComp.version < prefabVersion && isParent)
			{
				prefabComp.isDirty = true;
			}
		}
	}
}