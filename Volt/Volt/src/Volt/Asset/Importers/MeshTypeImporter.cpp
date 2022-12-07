#include "vtpch.h"
#include "MeshTypeImporter.h"

#include "FbxImporter.h"
#include "VTMeshImporter.h"

namespace Volt
{
	void MeshTypeImporter::Initialize()
	{
		myImporters[MeshFormat::Fbx] = CreateScope<FbxImporter>();
		myImporters[MeshFormat::VTMESH] = CreateScope<VTMeshImporter>();
	}

	void MeshTypeImporter::Shutdown()
	{
		myImporters.clear();
	}

	Ref<Mesh> MeshTypeImporter::ImportMesh(const std::filesystem::path& path)
	{
		return myImporters[FormatFromExtension(path)]->ImportMeshImpl(path);
	}

	Ref<Skeleton> MeshTypeImporter::ImportSkeleton(const std::filesystem::path& path)
	{
		return myImporters[FormatFromExtension(path)]->ImportSkeletonImpl(path);
	}

	Ref<Animation> MeshTypeImporter::ImportAnimation(const std::filesystem::path& path)
	{
		return myImporters[FormatFromExtension(path)]->ImportAnimationImpl(path);
	}

	MeshTypeImporter::MeshFormat MeshTypeImporter::FormatFromExtension(const std::filesystem::path& path)
	{
		auto ext = path.extension().string();

		if (ext == ".fbx" || ext == ".FBX")
		{
			return MeshFormat::Fbx;
		}
		else if (ext == ".gltf" || ext == ".glb")
		{
			return MeshFormat::GLTF;
		} 
		else if (ext == ".vtmesh")
		{
			return MeshFormat::VTMESH;
		}

		return MeshFormat::Other;
	}

}