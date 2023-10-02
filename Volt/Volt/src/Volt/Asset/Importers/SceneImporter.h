#pragma once

#include "Volt/Asset/Importers/AssetImporter.h"

#include "Volt/Utility/FileIO/YAMLStreamWriter.h"
#include "Volt/Utility/FileIO/YAMLStreamReader.h"

#include <entt.hpp>

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
	class IComponentTypeDesc;
	class IArrayTypeDesc;

	class SceneImporter : public AssetImporter
	{
	public:
		SceneImporter();
		~SceneImporter() override;

		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;

		void SerializeEntity(entt::entity id, const AssetMetadata& metadata, const Ref<Scene>& scene, YAMLStreamWriter& streamWriter) const;
		void DeserializeEntity(const Ref<Scene>& scene, const AssetMetadata& metadata, YAMLStreamReader& streamReader) const;
		void DeserializeMono(entt::entity id, const Ref<Scene>& scene, YAMLStreamReader& streamReader) const;

		[[nodiscard]] inline static const SceneImporter& Get() { return *s_instance; }
		[[nodiscard]] inline static const auto& GetTypeDeserializers() { return s_typeDeserializers; }

	private:
		inline static SceneImporter* s_instance = nullptr;

		void LoadSceneLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const;
		void SaveSceneLayers(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const;

		void SerializeClass(const uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, YAMLStreamWriter& streamWriter, bool isSubComponent) const;
		void SerializeArray(const uint8_t* data, const size_t offset, const IArrayTypeDesc* arrayDesc, YAMLStreamWriter& streamWriter) const;
		void SerializeMono(entt::entity id, const Ref<Scene>& scene, YAMLStreamWriter& streamWriter) const;

		void DeserializeClass(uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, YAMLStreamReader& streamReader) const;
		void DeserializeArray(uint8_t* data, const size_t offset, const IArrayTypeDesc* arrayDesc, YAMLStreamReader& streamReader) const;

		inline static std::unordered_map<std::type_index, std::function<void(YAMLStreamWriter&,	const uint8_t*, const size_t)>> s_typeSerializers;
		inline static std::unordered_map<std::type_index, std::function<void(YAMLStreamReader&,	uint8_t*, const size_t)>> s_typeDeserializers;
	};
}
