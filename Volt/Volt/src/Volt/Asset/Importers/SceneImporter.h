#pragma once

#include "Volt/Asset/Importers/AssetImporter.h"

#include <Wire/Wire.h>
#include <thread>

namespace GraphKey
{
	class Graph;
}

namespace Volt
{
	class Scene;
	class SceneImporter : public AssetImporter
	{
	public:
		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;

	private:
		void SerializeGraph(Ref<GraphKey::Graph> graph, const std::string& graphState, const Wire::Registry& registry, YAML::Emitter& out) const;
		void DeserializeGraph(Ref<GraphKey::Graph> graph, const YAML::Node& node) const;

		void SerializeEntity(Wire::EntityId id, const Wire::Registry& registry, const std::filesystem::path& targetDir) const;
		void DeserializeEntity(const std::filesystem::path& path, Wire::Registry& registry, Ref<Scene> scene) const;
	};
}