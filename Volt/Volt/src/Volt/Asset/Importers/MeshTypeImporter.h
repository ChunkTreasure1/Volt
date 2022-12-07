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

		static Ref<Mesh> ImportMesh(const std::filesystem::path& path);
		static Ref<Skeleton> ImportSkeleton(const std::filesystem::path& path);
		static Ref<Animation> ImportAnimation(const std::filesystem::path& path);

	protected:
		virtual Ref<Mesh> ImportMeshImpl(const std::filesystem::path& path) = 0;
		virtual Ref<Skeleton> ImportSkeletonImpl(const std::filesystem::path& path) = 0;
		virtual Ref<Animation> ImportAnimationImpl(const std::filesystem::path& path) = 0;

	private:
		enum class MeshFormat
		{
			Fbx,
			GLTF,
			VTMESH,
			Other
		};

		static MeshFormat FormatFromExtension(const std::filesystem::path& path);

		inline static std::unordered_map<MeshFormat, Scope<MeshTypeImporter>> myImporters;
	};
}