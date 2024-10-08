#pragma once

#include "Volt/Asset/SourceAssetImporters/ImportConfigs.h"

#include <AssetSystem/SourceAssetImporter.h>
#include <AssetSystem/SourceAssetImporterRegistry.h>

VT_DECLARE_LOG_CATEGORY(LogFbxSourceImporter, LogVerbosity::Trace);

namespace fbxsdk
{
	class FbxMesh;
	class FbxScene;
	class FbxString;
}

namespace Volt
{
	class Material;
	class Mesh;
	class Skeleton;
	class Animation;

	struct FbxVertex;
	struct FbxSkeletonContainer;

	class FbxSourceImporter final : public SourceAssetImporter
	{
	protected:
		Vector<Ref<Asset>> ImportInternal(const std::filesystem::path& filepath, const void* config, const SourceAssetUserImportData& userData) const override;
		SourceAssetFileInformation GetSourceFileInformation(const std::filesystem::path& filepath) const override;

	private:
		struct JointLink
		{
			uint32_t jointIndex;
			float weight;
		};

		using JointVertexLinkMap = std::unordered_multimap<uint32_t, JointLink>;

		void CreateVoltMeshFromFbxMesh(const fbxsdk::FbxMesh& fbxMesh, Ref<Mesh> destinationMesh, const Vector<Ref<Material>>& materials, const JointVertexLinkMap* jointVertexLinks) const;
		void CreateVoltSkeletonFromFbxSkeleton(const FbxSkeletonContainer& fbxSkeleton, Ref<Skeleton> destinationSkeleton) const;

		void FindJointVertexLinksAndSetupSkeleton(const fbxsdk::FbxMesh& fbxMesh, FbxSkeletonContainer& inOutSkeleton, JointVertexLinkMap& outVertexLinks) const;
		void CreateSubMeshFromVertexRange(Ref<Mesh> destinationMesh, const FbxVertex* vertices, size_t indexCount, const std::string& name) const;

		void CreateNonIndexedMesh(const fbxsdk::FbxMesh& fbxMesh, Vector<FbxVertex>& outVertices, const JointVertexLinkMap* jointVertexLinks) const;

		Ref<Animation> CreateAnimationFromFbxAnimation(fbxsdk::FbxScene* fbxScene, const FbxSkeletonContainer& fbxSkeleton, const fbxsdk::FbxString& animStackName, const MeshSourceImportConfig& importConfig, const SourceAssetUserImportData& userData) const;

		Vector<Ref<Asset>> ImportAsStaticMesh(fbxsdk::FbxScene* fbxScene, const MeshSourceImportConfig& importConfig, const SourceAssetUserImportData& userData) const;
		Vector<Ref<Asset>> ImportAsSkeletalMesh(fbxsdk::FbxScene* fbxScene, const MeshSourceImportConfig& importConfig, const SourceAssetUserImportData& userData) const;
		Vector<Ref<Asset>> ImportAsAnimation(fbxsdk::FbxScene* fbxScene, const MeshSourceImportConfig& importConfig, const SourceAssetUserImportData& userData) const;
	};

	VT_REGISTER_SOURCE_ASSET_IMPORTER(({ ".fbx", ".dxf", ".dae", ".obj", ".3ds" }), FbxSourceImporter);
}
