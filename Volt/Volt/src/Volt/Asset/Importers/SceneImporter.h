#pragma once

#include "Volt/Asset/Importers/AssetImporter.h"

#include <thread>
#include <typeindex>

namespace GraphKey
{
	class Graph;
	struct Node;
}

namespace Volt
{
	class Scene;
	class MonoScriptFieldCache;
	//class SceneImporter : public AssetImporter
	//{
	//public:
	//	SceneImporter();
	//	~SceneImporter();

	//	bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	//	void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;

	//	inline static auto& GetPropertySerializers() { return myPropertySerializer; }
	//	inline static auto& GetPropertyDeserializers() { return myPropertyDeserializer; }

	//	inline static auto& GetTypeIndexSerializers() { return myTypeIndexSerializeMap; }
	//	inline static auto& GetTypeIndexDeserializers() { return myTypeIndexDeserializeMap; }

	//	static void SerializeMono(entt::entity id, const MonoScriptFieldCache& scriptFieldCache, const Wire::Registry& registry, YAML::Emitter& out);
	//	static void DeserializeMono(entt::entity id, MonoScriptFieldCache& scriptFieldCache, const YAML::Node& node);
	//
	//private:
	//	void SerializeEntityLayer(entt::entity id, const Wire::Registry& registry, YAML::Emitter& out, const AssetMetadata& metadata, const Ref<Scene>& scene) const;
	//	void DeserializeEntityLayer(Wire::Registry& registry, const AssetMetadata& metadata, Ref<Scene> scene, const YAML::Node& node) const;

	//	void SaveLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const;
	//	void LoadLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const;

	//	bool CheckComponentDataMatch(const uint8_t* buf1, const uint8_t* buf2, Wire::ComponentRegistry::ComponentProperty prop) const;

	//	inline static std::unordered_map<Wire::ComponentRegistry::PropertyType, std::function<void(uint8_t*, uint32_t, Wire::ComponentRegistry::PropertyType, YAML::Emitter&)>> myPropertySerializer;
	//	inline static std::unordered_map<Wire::ComponentRegistry::PropertyType, std::function<void(uint8_t*, uint32_t, Wire::ComponentRegistry::PropertyType, const YAML::Node&)>> myPropertyDeserializer;
	//	inline static std::unordered_map<std::type_index, std::function<void(const std::any&, YAML::Emitter&)>> myTypeIndexSerializeMap;
	//	inline static std::unordered_map<std::type_index, std::function<void(std::any&, const YAML::Node&)>> myTypeIndexDeserializeMap;
	//};
}
