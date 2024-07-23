#pragma once

#include "MeshTypeImporter.h"

namespace Volt
{
	class ObjImporter : public MeshTypeImporter
	{
	public:
		ObjImporter() = default;

	protected:
		bool ImportMeshImpl(const std::filesystem::path&, Mesh& mesh) override { return false; };
		bool ImportSkeletonImpl(const std::filesystem::path&, Skeleton& skeleton) override { return false; }
		bool ImportAnimationImpl(const std::filesystem::path&, Ref<Skeleton>, Animation& animation) override { return false; }

		void ExportMeshImpl(Vector<Ref<Mesh>> assets, const std::filesystem::path& path) override;
		void ExportSkeletonImpl(Vector<Ref<Skeleton>>, const std::filesystem::path&) override {};
		void ExportAnimationImpl(Vector<Ref<Animation>>, const std::filesystem::path&) override {};
	};
}
