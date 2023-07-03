#include "vtpch.h"
#include "MaterialTable.h"

#include "Volt/Asset/Mesh/SubMaterial.h"

#include "Volt/Rendering/Buffer/ShaderStorageBufferSet.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/RendererStructs.h"

#include "Volt/Rendering/Texture/TextureTable.h"
#include "Volt/Rendering/Texture/Texture2D.h"

namespace Volt
{
	MaterialTable::MaterialTable()
	{
		myStorageBuffer = ShaderStorageBufferSet::Create(Renderer::GetFramesInFlightCount());
		myStorageBuffer->Add<GPUMaterial>(Sets::MAINBUFFERS, Bindings::MATERIAL_TABLE, Renderer::MAX_MATERIALS , MemoryUsage::CPUToGPU);

		myGPUMaterials.resize(Renderer::MAX_MATERIALS);
		myIsDirty = std::vector<bool>(Renderer::GetFramesInFlightCount(), false);

		myCurrentIndex = myStartIndex;
	}
	
	MaterialTable::~MaterialTable()
	{
		myMaterialIndexMap.clear();
		myStorageBuffer = nullptr;
	}

	const uint32_t MaterialTable::AddMaterial(SubMaterial* material)
	{
		uint32_t binding = 0;

		if (!myAvailableIndices.empty())
		{
			binding = myAvailableIndices.back();
			myAvailableIndices.pop_back();
		}
		else
		{
			binding = myCurrentIndex++;
		}

		myMaterialIndexMap.emplace(material, binding);
		UpdateMaterial(material);

		SetDirty(true);

		return binding;
	}

	void MaterialTable::RemoveMaterial(SubMaterial* material)
	{
		if (myMaterialIndexMap.contains(material))
		{
			myMaterialIndexMap.erase(material);
		}

		SetDirty(true);
	}

	void MaterialTable::UpdateMaterial(SubMaterial* material)
	{
		if (!myMaterialIndexMap.contains(material))
		{
			return;
		}

		const uint32_t index = myMaterialIndexMap.at(material);
		auto& gpuMat = myGPUMaterials.at(index);

		auto materialPtr = material;

		// This assumes that albedo is in slot 0, normal in slot 1 and material in slot 2

		const auto& bindlessData = Renderer::GetBindlessData();
		const auto& textures = materialPtr->GetTextures();
		
		if (textures.contains("albedo") && textures.at("albedo"))
		{
			gpuMat.albedoTexture = bindlessData.textureTable->GetBindingFromTexture(textures.at("albedo")->GetImage());
		}

		if (textures.contains("normal") && textures.at("normal"))
		{
			gpuMat.normalTexture = bindlessData.textureTable->GetBindingFromTexture(textures.at("normal")->GetImage());
		}

		if (textures.contains("material") && textures.at("material"))
		{
			gpuMat.materialTexture = bindlessData.textureTable->GetBindingFromTexture(textures.at("material")->GetImage());
		}

		const auto& materialData = materialPtr->GetMaterialData();
		gpuMat.color = materialData.color;
		gpuMat.emissiveColor = materialData.emissiveColor;
		gpuMat.emissiveStrength = materialData.emissiveStrength;
		gpuMat.roughness = materialData.roughness;
		gpuMat.metalness = materialData.metalness;
		gpuMat.normalStrength = materialData.normalStrength;
		gpuMat.materialFlags = static_cast<uint32_t>(materialPtr->GetFlags());

		SetDirty(true);
	}

	void MaterialTable::UpdateMaterialData(uint32_t index)
	{
		if (!myIsDirty.at(index))
		{
			return;
		}

		myIsDirty.at(index) = false;

		auto buffer = myStorageBuffer->Get(Sets::MAINBUFFERS, Bindings::MATERIAL_TABLE, index);
		auto* mappedPtr = buffer->Map<GPUMaterial>();

		memcpy_s(mappedPtr, sizeof(GPUMaterial) * Renderer::MAX_MATERIALS, myGPUMaterials.data(), std::min((uint32_t)myGPUMaterials.size(), Renderer::MAX_MATERIALS) * sizeof(GPUMaterial));
		buffer->Unmap();
	}

	const SubMaterial* MaterialTable::GetMaterialFromIndex(uint32_t index)
	{
		for (const auto& [material, matIndex] : myMaterialIndexMap)
		{
			if (index == matIndex)
			{
				return material;
			}
		}

		return nullptr;
	}

	const uint32_t MaterialTable::GetIndexFromMaterial(SubMaterial* material)
	{
		return myMaterialIndexMap.at(material);
	}

	const Ref<ShaderStorageBuffer> MaterialTable::GetStorageBuffer(uint32_t index) const
	{
		return myStorageBuffer->Get(Sets::MAINBUFFERS, Bindings::MATERIAL_TABLE, index);
	}

	const Ref<ShaderStorageBufferSet> MaterialTable::GetStorageBufferSet() const
	{
		return myStorageBuffer;
	}

	Ref<MaterialTable> MaterialTable::Create()
	{
		return CreateRef<MaterialTable>();
	}

	void MaterialTable::SetDirty(bool state)
	{
		for (auto&& val : myIsDirty)
		{
			val = state;
		}
	}
}
