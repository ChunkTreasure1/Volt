#pragma once

#include "MeshTypeImporter.h"

namespace Volt
{
	class ObjImporter : public MeshTypeImporter
	{
	public:
		ObjImporter() = default;

	protected:
		Ref<Mesh> ImportMeshImpl(const std::filesystem::path& path) override { return nullptr; };
		Ref<Skeleton> ImportSkeletonImpl(const std::filesystem::path&) override { return nullptr; }
		Ref<Animation> ImportAnimationImpl(const std::filesystem::path&, Ref<Skeleton> targetSkeleton) override { return nullptr; }

		void ExportMeshImpl(std::vector<Ref<Mesh>> assets, const std::filesystem::path& path) override;
		void ExportSkeletonImpl(std::vector<Ref<Skeleton>> assets, const std::filesystem::path& path) override {};
		void ExportAnimationImpl(std::vector<Ref<Animation>> assets, const std::filesystem::path& path) override {};
	};
}
