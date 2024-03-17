#pragma once

#include "Volt/Asset/Asset.h"

#include <unordered_map>

namespace Volt
{
	class Mesh;
	class Skeleton;
	class Animation;

	class MeshTypeImporter
	{
	public:
		virtual ~MeshTypeImporter() = default;

		static void Initialize();
		static void Shutdown();

		static void ImportMesh(const std::filesystem::path& path, Ref<Mesh>& mesh);
		static void ImportSkeleton(const std::filesystem::path& path, Ref<Skeleton>& skeleton);
		static void ImportAnimation(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Ref<Animation>& animation);

		static void ExportMesh(std::vector<Ref<Mesh>> assets, const std::filesystem::path& path);
		static void ExportSkeleton(std::vector<Ref<Skeleton>> assets, const std::filesystem::path& path);
		static void ExportAnimation(std::vector<Ref<Animation>> assets, const std::filesystem::path& path);

	protected:
		virtual void ImportMeshImpl(const std::filesystem::path& path, Ref<Mesh>& mesh) = 0;
		virtual void ImportSkeletonImpl(const std::filesystem::path& path, Ref<Skeleton>& skeleton) = 0;
		virtual void ImportAnimationImpl(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Ref<Animation>& animation) = 0;

		virtual void ExportMeshImpl(std::vector<Ref<Mesh>>, const std::filesystem::path&) {};
		virtual void ExportSkeletonImpl(std::vector<Ref<Skeleton>>, const std::filesystem::path&) {};
		virtual void ExportAnimationImpl(std::vector<Ref<Animation>>, const std::filesystem::path&) {};

	private:
		enum class MeshFormat
		{
			Fbx,
			GLTF,
			VTMESH,
			OBJ,
			Other
		};

		static MeshFormat FormatFromExtension(const std::filesystem::path& path);

		inline static std::unordered_map<MeshFormat, Scope<MeshTypeImporter>> myImporters;
	};
}
