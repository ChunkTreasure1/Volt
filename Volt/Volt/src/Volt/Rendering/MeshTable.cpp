#include "vtpch.h"
#include "MeshTable.h"

#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Rendering/Buffer/ShaderStorageBufferSet.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/Shader/ShaderUtility.h"

namespace Volt
{
	MeshTable::MeshTable()
	{
		myStorageBuffer = ShaderStorageBufferSet::Create(Renderer::GetFramesInFlightCount());
		myStorageBuffer->Add<GPUMaterial>(Sets::MAINBUFFERS, Bindings::MESH_TABLE, Renderer::MAX_MESHES, MemoryUsage::CPUToGPU);

		myGPUMeshes.resize(Renderer::MAX_MESHES);
		myIsDirty = std::vector<bool>(Renderer::GetFramesInFlightCount(), false);
	}

	MeshTable::~MeshTable()
	{
		myMeshMap.clear();
		myStorageBuffer = nullptr;
	}

	const uint32_t MeshTable::AddMesh(Mesh* mesh, const uint32_t subMeshIndex)
	{
		uint32_t binding = 0;

		const size_t hash = GetHash(mesh, subMeshIndex);

		if (!myAvailableIndices.empty())
		{
			binding = myAvailableIndices.back();
			myAvailableIndices.pop_back();
		}
		else
		{
			binding = myCurrentIndex++;
		}

		myMeshMap.emplace(hash, binding);
		UpdateMesh(mesh, subMeshIndex);
		SetDirty(true);

		return binding;
	}

	void MeshTable::RemoveMesh(Mesh* mesh, const uint32_t subMeshIndex)
	{
		const size_t hash = GetHash(mesh, subMeshIndex);

		if (myMeshMap.contains(hash))
		{
			myMeshMap.erase(hash);
		}

		SetDirty(true);
	}

	void MeshTable::UpdateMesh(Mesh* mesh, const uint32_t subMeshIndex)
	{
		const size_t hash = GetHash(mesh, subMeshIndex);

		if (!myMeshMap.contains(hash))
		{
			return;
		}

		const uint32_t index = myMeshMap.at(hash);
		auto& gpuMesh = myGPUMeshes.at(index);
	
		gpuMesh = mesh->GetGPUMeshes().at(subMeshIndex);

		SetDirty(true);
	}

	void MeshTable::UpdateMeshData(uint32_t index)
	{
		if (!myIsDirty.at(index))
		{
			return;
		}

		myIsDirty.at(index) = false;

		auto buffer = myStorageBuffer->Get(Sets::MAINBUFFERS, Bindings::MESH_TABLE, index);
		auto* mappedPtr = buffer->Map<GPUMesh>();

		memcpy_s(mappedPtr, sizeof(GPUMesh) * Renderer::MAX_MESHES, myGPUMeshes.data(), std::min((uint32_t)myGPUMeshes.size(), Renderer::MAX_MESHES) * sizeof(GPUMesh));
		buffer->Unmap();
	}

	const uint32_t MeshTable::GetMeshIndex(Mesh* mesh, const uint32_t subMeshIndex)
	{
		//const size_t hash = GetHash(mesh, subMeshIndex);
		return 0; /*myMeshMap.at(hash);*/
	}

	const Ref<ShaderStorageBuffer> MeshTable::GetStorageBuffer(uint32_t index) const
	{
		return myStorageBuffer->Get(Sets::MAINBUFFERS, Bindings::MESH_TABLE, index);
	}

	const Ref<ShaderStorageBufferSet> MeshTable::GetStorageBufferSet() const
	{
		return myStorageBuffer;
	}

	Ref<MeshTable> MeshTable::Create()
	{
		return CreateRef<MeshTable>();
	}

	void MeshTable::SetDirty(bool state)
	{
		for (auto&& val : myIsDirty)
		{
			val = state;
		}
	}

	const size_t MeshTable::GetHash(Mesh* mesh, const uint32_t subMeshIndex)
	{
		size_t meshHash = std::hash<void*>()(mesh);
		meshHash = Utility::HashCombine(meshHash, std::hash<uint32_t>()(subMeshIndex));

		return meshHash;
	}
}
