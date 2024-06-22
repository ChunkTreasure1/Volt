#pragma once

#include "MeshTypeImporter.h"
#include "Volt/Rendering/Vertex.h"

struct ufbx_mesh;
struct ufbx_scene;
struct ufbx_node;

namespace Volt
{
	class Mesh;
	class Skeleton;
	class FbxImporter : public MeshTypeImporter
	{
	public:
		FbxImporter() = default;

	protected:
		bool ImportMeshImpl(const std::filesystem::path& path, Mesh& destMesh) override;
		bool ImportSkeletonImpl(const std::filesystem::path& path, Skeleton& dstSkeleton) override;
		bool ImportAnimationImpl(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Animation& dstAnimation) override;

		void ExportMeshImpl(std::vector<Ref<Mesh>> assets, const std::filesystem::path& path) override;
		void ExportSkeletonImpl(std::vector<Ref<Skeleton>> assets, const std::filesystem::path&) override {};
		void ExportAnimationImpl(std::vector<Ref<Animation>> assets, const std::filesystem::path&) override {};

	private:
		ufbx_scene* LoadScene(const std::filesystem::path& path);

		void ReadMesh(Mesh& dstMesh, Skeleton& skeleton, ufbx_mesh* mesh);
		
		void GatherSkeleton(Skeleton& skeleton, const ufbx_node* currentNode, int32_t parentIndex);
		void ReadSkinningData(Skeleton& skeleton, const ufbx_scene* scene);
	};
}
