#include "vtpch.h"
#include "AnimationGraphImporter.h"

#include "Volt/Asset/Importers/SceneImporter.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Asset/Animation/AnimationGraphAsset.h"

#include "Volt/Project/ProjectManager.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"

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

		YAMLFileStreamReader streamReader{};
		if (!streamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		streamReader.EnterScope("AnimationGraph");

		// Graph base
		{
			std::string state = streamReader.ReadAtKey("state", std::string(""));
			AssetHandle skeletonHandle = streamReader.ReadAtKey("skeleton", AssetHandle(0));

			animGraph->mySkeletonHandle = skeletonHandle;
			animGraph->myGraphState = state;
		}

		// Graph
		{
			streamReader.EnterScope("Graph");
			GraphKey::Graph::Deserialize(animGraph, streamReader);
			streamReader.ExitScope();
		}

		streamReader.ExitScope();

		animGraph->SetSkeletonHandle(animGraph->mySkeletonHandle);
		return true;
	}

	void AnimationGraphImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		const Ref<AnimationGraphAsset> animGraph = std::reinterpret_pointer_cast<AnimationGraphAsset>(asset);

		YAMLFileStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };
		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("AnimationGraph");

		streamWriter.SetKey("state", animGraph->myGraphState);
		streamWriter.SetKey("skeleton", animGraph->mySkeletonHandle);

		GraphKey::Graph::Serialize(animGraph, streamWriter);

		streamWriter.EndMap();
		streamWriter.EndMap();

		streamWriter.WriteToDisk();
	}
}
