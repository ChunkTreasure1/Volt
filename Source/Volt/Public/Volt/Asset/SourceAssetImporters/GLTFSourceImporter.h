#pragma once

#include "Volt/Asset/SourceAssetImporters/ImportConfigs.h"

#include <AssetSystem/SourceAssetImporter.h>
#include <AssetSystem/SourceAssetImporterRegistry.h>

#include <LogModule/LogCategory.h>

VT_DECLARE_LOG_CATEGORY(LogGLTFSourceImporter, LogVerbosity::Trace);

namespace tinygltf
{
	class Model;
	class Node;
	struct Mesh;
}

namespace Volt
{
	class Mesh;
	class Material;

	class GLTFSourceImporter final : public SourceAssetImporter
	{
	protected:
		Vector<Ref<Asset>> ImportInternal(const std::filesystem::path& filepath, const void* config, const SourceAssetUserImportData& userData);
	
	private:
		void CreateVoltMeshFromGLTFMesh(const tinygltf::Mesh& gltfMesh, const tinygltf::Node& gltfNode, const tinygltf::Model& gltfModel, Ref<Mesh> destinationMesh, const Vector<Ref<Material>>& materials) const;

		Vector<Ref<Asset>> ImportAsStaticMesh(tinygltf::Model& gltfModel, const MeshSourceImportConfig importConfig, const SourceAssetUserImportData& userData);
	};

	VT_REGISTER_SOURCE_ASSET_IMPORTER(({ ".gltf", ".glb" }), GLTFSourceImporter);
}
