#include "vtpch.h"
#include "MeshTypeImporter.h"

#include "FbxImporter.h"
#include "VTMeshImporter.h"
#include "ObjImporter.h"
#include "GLTFImporter.h"

namespace Volt
{
	void MeshTypeImporter::Initialize()
	{
		myImporters[MeshFormat::Fbx] = CreateScope<FbxImporter>();
		myImporters[MeshFormat::VTMESH] = CreateScope<VTMeshImporter>();
		myImporters[MeshFormat::OBJ] = CreateScope<ObjImporter>();
		myImporters[MeshFormat::GLTF] = CreateScope<GLTFImporter>();
	}

	void MeshTypeImporter::Shutdown()
	{
		myImporters.clear();
	}

	bool MeshTypeImporter::ImportMesh(const std::filesystem::path& path, Mesh& dstMesh)
	{
		return myImporters[FormatFromExtension(path)]->ImportMeshImpl(path, dstMesh);
	}

	bool MeshTypeImporter::ImportSkeleton(const std::filesystem::path& path, Skeleton& dstSkeleton)
	{
		return myImporters[FormatFromExtension(path)]->ImportSkeletonImpl(path, dstSkeleton);
	}

	bool MeshTypeImporter::ImportAnimation(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Animation& dstAnimation)
	{
		return myImporters[FormatFromExtension(path)]->ImportAnimationImpl(path, targetSkeleton, dstAnimation);
	}

	void MeshTypeImporter::ExportMesh(std::vector<Ref<Mesh>> assets, const std::filesystem::path& path)
	{
		myImporters[FormatFromExtension(path)]->ExportMeshImpl(assets, path);
	}

	void MeshTypeImporter::ExportSkeleton(std::vector<Ref<Skeleton>> assets, const std::filesystem::path& path)
	{
		myImporters[FormatFromExtension(path)]->ExportSkeletonImpl(assets, path);
	}

	void MeshTypeImporter::ExportAnimation(std::vector<Ref<Animation>> assets, const std::filesystem::path& path)
	{
		myImporters[FormatFromExtension(path)]->ExportAnimationImpl(assets, path);
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
		else if (ext == ".obj" || ext == ".OBJ")
		{
			return MeshFormat::OBJ;
		}
		else if (ext == ".vtmesh")
		{
			return MeshFormat::VTMESH;
		}
		else if (ext == ".vtnavmesh")
		{
			return MeshFormat::VTMESH;
		}

		return MeshFormat::Other;
	}

}
