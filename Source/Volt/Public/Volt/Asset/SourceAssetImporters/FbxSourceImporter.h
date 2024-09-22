#pragma once

#include "Volt/Asset/SourceAssetImporters/ImportConfigs.h"

#include <AssetSystem/SourceAssetImporter.h>
#include <AssetSystem/SourceAssetImporterRegistry.h>

VT_DECLARE_LOG_CATEGORY(LogFbxSourceImporter, LogVerbosity::Trace);

namespace fbxsdk
{
	class FbxMesh;
	class FbxScene;
}

namespace Volt
{
	class Material;
	class Mesh;
	struct FbxVertex;

	class FbxSourceImporter final : public SourceAssetImporter
	{
	protected:
		Vector<Ref<Asset>> ImportInternal(const std::filesystem::path& filepath, const void* config, const SourceAssetUserImportData& userData);

	private:
		void CreateVoltMeshFromFbxMesh(const fbxsdk::FbxMesh& fbxMesh, Ref<Mesh> destinationMesh, const Vector<Ref<Material>>& materials);
		void CreateSubMeshFromVertexRange(Ref<Mesh> destinationMesh, const FbxVertex* vertices, size_t indexCount, const std::string& name);

		Vector<Ref<Asset>> ImportAsStaticMesh(fbxsdk::FbxScene* fbxScene, const MeshSourceImportConfig& importConfig);
	};

	VT_REGISTER_SOURCE_ASSET_IMPORTER(({ ".fbx", ".dxf", ".dae", ".obj", ".3ds" }), FbxSourceImporter);
}
