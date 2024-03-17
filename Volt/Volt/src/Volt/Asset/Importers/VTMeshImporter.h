#pragma once

#include "MeshTypeImporter.h"

namespace Volt
{
	class VTMeshImporter : public MeshTypeImporter
	{
	public:
		VTMeshImporter() = default;

	protected:
		void ImportMeshImpl(const std::filesystem::path& path, Ref<Mesh>& mesh) override;
		void ImportSkeletonImpl(const std::filesystem::path&, Ref<Skeleton>& skeleton) override { }
		void ImportAnimationImpl(const std::filesystem::path&, Ref<Skeleton> targetSkeleton, Ref<Animation>& animation) override { }

	private:
		bool IsValid(uint32_t subMeshCount, uint32_t vertexCount, uint32_t indexCount, size_t totalSize) const;
	};
}
