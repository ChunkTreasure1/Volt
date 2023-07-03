#include "vtpch.h"
#include "AnimationGraphImporter.h"

#include "Volt/Asset/Importers/SceneImporter.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Asset/Animation/AnimationGraphAsset.h"

#include "Volt/Project/ProjectManager.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/SerializationMacros.h"

namespace Volt
{
	bool AnimationGraphImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<AnimationGraphAsset>();
		Ref<AnimationGraphAsset> animGraph = std::reinterpret_pointer_cast<AnimationGraphAsset>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream input(filePath, std::ios::binary | std::ios::in);
		if (!input.is_open())
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << input.rdbuf();
		input.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML with error {1}! Please correct it!", metadata.filePath, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node graphNode = root["AnimationGraph"];

		// Graph base
		{
			std::string state;
			VT_DESERIALIZE_PROPERTY(state, state, graphNode, std::string(""));

			AssetHandle characterHandle;
			VT_DESERIALIZE_PROPERTY(character, characterHandle, graphNode, AssetHandle(0));

			animGraph->myAnimatedCharacter = characterHandle;
			animGraph->myGraphState = state;
		}

		// Graph
		{
			YAML::Node graphSaveNode = graphNode["Graph"];
			if (graphNode)
			{
				GraphKey::Graph::Deserialize(animGraph, graphSaveNode);
			}
		}
		return true;
	}

	void AnimationGraphImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<AnimationGraphAsset> animGraph = std::reinterpret_pointer_cast<AnimationGraphAsset>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "AnimationGraph" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(state, animGraph->myGraphState, out);
			VT_SERIALIZE_PROPERTY(character, animGraph->myAnimatedCharacter, out);
			GraphKey::Graph::Serialize(animGraph, out);
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetFilesystemPath(metadata.filePath));
		fout << out.c_str();
		fout.close();
	}
}
