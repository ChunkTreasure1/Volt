#pragma once

#include "MeshTypeImporter.h"

namespace Volt
{
	class VTMeshImporter : public MeshTypeImporter
	{
	public:
		VTMeshImporter() = default;

	protected:
		Ref<Mesh> ImportMeshImpl(const std::filesystem::path& path);
		Ref<Skeleton> ImportSkeletonImpl(const std::filesystem::path&) override { return nullptr; }
		Ref<Animation> ImportAnimationImpl(const std::filesystem::path&) override { return nullptr; }

	private:
		bool IsValid(uint32_t subMeshCount, uint32_t vertexCount, uint32_t indexCount, size_t totalSize) const;
	};
}