#pragma once

#include "MeshTypeImporter.h"
#include "Volt/Rendering/Vertex.h"

#include <TGAFbx.h>

namespace Volt
{
	class Mesh;
	class Skeleton;
	class FbxImporter : public MeshTypeImporter
	{
	public:
		FbxImporter() = default;

	protected:
		bool ImportMeshImpl(const std::filesystem::path& path, Mesh& dstMesh) override;
		bool ImportSkeletonImpl(const std::filesystem::path& path, Skeleton& dstSkeleton) override;
		bool ImportAnimationImpl(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Animation& dstAnimation) override;

		void ExportMeshImpl(std::vector<Ref<Mesh>> assets, const std::filesystem::path& path) override;
		void ExportSkeletonImpl(std::vector<Ref<Skeleton>> assets, const std::filesystem::path&) override {};
		void ExportAnimationImpl(std::vector<Ref<Animation>> assets, const std::filesystem::path&) override {};

	private:
		struct VertexDuplicateData
		{
			uint32_t index;
			size_t hash;
		};

		void ProcessSkeleton(Skeleton& skeleton, const std::vector<TGA::FBX::Skeleton::Bone>& bones, uint32_t currentIndex);
	};
}
