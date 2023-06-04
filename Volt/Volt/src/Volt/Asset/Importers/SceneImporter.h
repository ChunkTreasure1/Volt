#pragma once

#include "Volt/Asset/Importers/AssetImporter.h"

#include <Wire/Wire.h>
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
	class SceneImporter : public AssetImporter
	{
	public:
		SceneImporter();
		~SceneImporter();

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;

		inline static auto& GetPropertySerializers() { return myPropertySerializer; }
		inline static auto& GetPropertyDeserializers() { return myPropertyDeserializer; }

		inline static auto& GetTypeIndexSerializers() { return myTypeIndexSerializeMap; }
		inline static auto& GetTypeIndexDeserializers() { return myTypeIndexDeserializeMap; }

		static void SerializeMono(Wire::EntityId id, const MonoScriptFieldCache& scriptFieldCache, const Wire::Registry& registry, YAML::Emitter& out);
		static void DeserializeMono(Wire::EntityId id, MonoScriptFieldCache& scriptFieldCache, const YAML::Node& node);
	
	private:
		void SerializeEntity(Wire::EntityId id, const Wire::Registry& registry, Ref<Scene> scene, const std::filesystem::path& targetDir) const;
		void DeserializeEntity(const std::filesystem::path& path, Wire::Registry& registry, Ref<Scene> scene) const;

		void SerializeEntityLayer(Wire::EntityId id, const Wire::Registry& registry, YAML::Emitter& out, const Ref<Scene>& scene) const;
		void DeserializeEntityLayer(Wire::Registry& registry, Ref<Scene> scene, const YAML::Node& node) const;

		void SaveLayers(const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const;
		void LoadLayers(const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const;

		bool CheckComponentDataMatch(const uint8_t* buf1, const uint8_t* buf2, Wire::ComponentRegistry::ComponentProperty prop) const;

		inline static std::unordered_map<Wire::ComponentRegistry::PropertyType, std::function<void(uint8_t*, uint32_t, Wire::ComponentRegistry::PropertyType, YAML::Emitter&)>> myPropertySerializer;
		inline static std::unordered_map<Wire::ComponentRegistry::PropertyType, std::function<void(uint8_t*, uint32_t, Wire::ComponentRegistry::PropertyType, const YAML::Node&)>> myPropertyDeserializer;
		inline static std::unordered_map<std::type_index, std::function<void(const std::any&, YAML::Emitter&)>> myTypeIndexSerializeMap;
		inline static std::unordered_map<std::type_index, std::function<void(std::any&, const YAML::Node&)>> myTypeIndexDeserializeMap;
	};
}
