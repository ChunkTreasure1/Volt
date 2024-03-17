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
		void ImportMeshImpl(const std::filesystem::path& path, Ref<Mesh>& mesh) override;
		void ImportSkeletonImpl(const std::filesystem::path& path, Ref<Skeleton>& skeleton) override;
		void ImportAnimationImpl(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Ref<Animation>& animation) override;

		void ExportMeshImpl(std::vector<Ref<Mesh>> assets, const std::filesystem::path& path) override;
		void ExportSkeletonImpl(std::vector<Ref<Skeleton>> assets, const std::filesystem::path&) override {};
		void ExportAnimationImpl(std::vector<Ref<Animation>> assets, const std::filesystem::path&) override {};

	private:
		struct VertexDuplicateData
		{
			uint32_t index;
			size_t hash;
		};

		void ProcessSkeleton(Ref<Skeleton> skeleton, const std::vector<TGA::FBX::Skeleton::Bone>& bones, uint32_t currentIndex);
	};
}
