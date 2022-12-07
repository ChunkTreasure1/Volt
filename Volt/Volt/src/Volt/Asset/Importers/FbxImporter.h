#pragma once

#include "MeshTypeImporter.h"

#include "Volt/Rendering/Vertex.h"

#include <fbxsdk.h>

namespace Volt
{
	class Mesh;
	class Skeleton;
	class FbxImporter : public MeshTypeImporter
	{
	public:
		FbxImporter() = default;

	protected:
		Ref<Mesh> ImportMeshImpl(const std::filesystem::path& path) override;
		Ref<Skeleton> ImportSkeletonImpl(const std::filesystem::path& path) override;
		Ref<Animation> ImportAnimationImpl(const std::filesystem::path& path) override;

	private:
		void ProcessMesh(FbxNode* fbxMesh, Ref<Skeleton> skeleton, FbxScene* aScene, std::unordered_multimap<uint32_t, std::pair<uint32_t, float>>& aControlPointWeights, Ref<Mesh> mesh);
		void FetchGeometryNodes(FbxNode* node, std::vector<FbxNode*>& outNodes);

		void GetElementMappingData(fbxsdk::FbxLayerElementTemplate<FbxVector4>* element, int32_t ctrlPointIndex, int32_t polyIndex, FbxVector4& outData);

		void ProcessSkeletonHierarchy(FbxNode* aNode, Ref<Skeleton> aSkeleton);
		void ProcessSkeletonHierarchyRecursively(FbxNode* aNode, int32_t aIndex, int32_t aParent, Ref<Skeleton> aSkeleton);
		void ProcessJoints(FbxNode* aNode, std::unordered_multimap<uint32_t, std::pair<uint32_t, float>>& aControlPointWeights, Ref<Skeleton> aSkeleton);

		uint32_t FindJointIndexByName(const std::string& aName, Ref<Skeleton> aSkeleton);
	};
}