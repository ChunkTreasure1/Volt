#pragma once

#include "MeshTypeImporter.h"

namespace Volt
{
	class ObjImporter : public MeshTypeImporter
	{
	public:
		ObjImporter() = default;

	protected:
		void ImportMeshImpl(const std::filesystem::path&, Ref<Mesh>&) override {};
		void ImportSkeletonImpl(const std::filesystem::path&, Ref<Skeleton>&) override {}
		void ImportAnimationImpl(const std::filesystem::path&, Ref<Skeleton>, Ref<Animation>&) override {}

		void ExportMeshImpl(std::vector<Ref<Mesh>> assets, const std::filesystem::path& path) override;
		void ExportSkeletonImpl(std::vector<Ref<Skeleton>>, const std::filesystem::path&) override {};
		void ExportAnimationImpl(std::vector<Ref<Animation>>, const std::filesystem::path&) override {};
	};
}
