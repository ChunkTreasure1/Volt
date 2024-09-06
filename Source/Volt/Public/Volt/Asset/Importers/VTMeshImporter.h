#pragma once

#include "MeshTypeImporter.h"

namespace Volt
{
	class VTMeshImporter : public MeshTypeImporter
	{
	public:
		VTMeshImporter() = default;

	protected:
		bool ImportMeshImpl(const std::filesystem::path& path, Mesh& dstMesh) override;
		bool ImportSkeletonImpl(const std::filesystem::path&, Skeleton& dstSkeleton) override { return false; }
		bool ImportAnimationImpl(const std::filesystem::path&, Ref<Skeleton> targetSkeleton, Animation& dstAnimation) override { return false; }

	private:
		bool IsValid(uint32_t subMeshCount, uint32_t vertexCount, uint32_t indexCount, size_t totalSize) const;
	};
}
