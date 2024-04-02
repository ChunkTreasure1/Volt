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

		static bool ImportMesh(const std::filesystem::path& path, Mesh& dstMesh);
		static bool ImportSkeleton(const std::filesystem::path& path, Skeleton& dstSkeleton);
		static bool ImportAnimation(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Animation& dstAnimation);

		static void ExportMesh(std::vector<Ref<Mesh>> assets, const std::filesystem::path& path);
		static void ExportSkeleton(std::vector<Ref<Skeleton>> assets, const std::filesystem::path& path);
		static void ExportAnimation(std::vector<Ref<Animation>> assets, const std::filesystem::path& path);

	protected:
		virtual bool ImportMeshImpl(const std::filesystem::path& path, Mesh& dstMesh) = 0;
		virtual bool ImportSkeletonImpl(const std::filesystem::path& path, Skeleton& dstSkeleton) = 0;
		virtual bool ImportAnimationImpl(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Animation& dstAnimation) = 0;

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
