#pragma once

#include "Volt/Asset/Serializers/AssetSerializer.h"

#include "Volt/Scene/Entity.h"

#include <typeindex>

class YAMLMemoryStreamWriter;
class YAMLMemoryStreamReader;

namespace Volt
{
	class Scene;
	class IArrayTypeDesc;
	class MonoScriptFieldCache;
	class IComponentTypeDesc;

	class SceneSerializer : public AssetSerializer
	{
	public:
		SceneSerializer();
		~SceneSerializer() override;

		void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
		bool Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const override;

		void SerializeEntity(entt::entity id, const AssetMetadata& metadata, const Ref<Scene>& scene, YAMLMemoryStreamWriter& streamWriter) const;
		void DeserializeEntity(const Ref<Scene>& scene, const AssetMetadata& metadata, YAMLMemoryStreamReader& streamReader) const;
		void DeserializeMono(entt::entity id, const Ref<Scene>& scene, YAMLMemoryStreamReader& streamReader) const;

		static SceneSerializer& Get() { return *s_instance; }

	private:
		inline static SceneSerializer* s_instance = nullptr;

		void SerializeWorldEngine(const Ref<Scene>& scene, YAMLMemoryStreamWriter& streamWriter) const;
		void DeserializeWorldEngine(const Ref<Scene>& scene, YAMLMemoryStreamReader& streamReader) const;

		void SerializeEntities(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const;
		void LoadCellEntities(const AssetMetadata& metadata, const Ref<Scene>& scene, const std::filesystem::path& sceneDirectory) const;

		Entity CreateEntityFromUUIDThreadSafe(EntityID entityId, const Ref<Scene>& scene) const;

		void SerializeClass(const uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, YAMLMemoryStreamWriter& streamWriter, bool isSubComponent) const;
		void SerializeArray(const uint8_t* data, const size_t offset, const IArrayTypeDesc* arrayDesc, YAMLMemoryStreamWriter& streamWriter) const;
		void SerializeMono(entt::entity id, const Ref<Scene>& scene, YAMLMemoryStreamWriter& streamWriter) const;

		void DeserializeClass(uint8_t* data, const size_t offset, const IComponentTypeDesc* compDesc, YAMLMemoryStreamReader& streamReader) const;
		void DeserializeArray(uint8_t* data, const size_t offset, const IArrayTypeDesc* arrayDesc, YAMLMemoryStreamReader& streamReader) const;

		inline static std::unordered_map<std::type_index, std::function<void(YAMLMemoryStreamWriter&, const uint8_t*, const size_t)>> s_typeSerializers;
		inline static std::unordered_map<std::type_index, std::function<void(YAMLMemoryStreamReader&, uint8_t*, const size_t)>> s_typeDeserializers;
	};
}
