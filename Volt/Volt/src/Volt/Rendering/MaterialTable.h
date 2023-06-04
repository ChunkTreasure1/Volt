#pragma once

#include "Volt/Rendering/RendererStructs.h"

#include <unordered_map>

namespace Volt
{
	class SubMaterial;
	class ShaderStorageBufferSet;
	class ShaderStorageBuffer;
	class MaterialTable
	{
	public:
		MaterialTable();
		~MaterialTable();

		const uint32_t AddMaterial(SubMaterial* material);
		void RemoveMaterial(SubMaterial* material);
		void UpdateMaterial(SubMaterial* material);

		void UpdateMaterialData(uint32_t index);

		const SubMaterial* GetMaterialFromIndex(uint32_t index);
		const uint32_t GetIndexFromMaterial(SubMaterial* material);

		const Ref<ShaderStorageBuffer> GetStorageBuffer(uint32_t index) const;
		const Ref<ShaderStorageBufferSet> GetStorageBufferSet() const;

		static Ref<MaterialTable> Create();

	private:
		void SetDirty(bool state);

		std::vector<bool> myIsDirty;

		std::vector<uint32_t> myAvailableIndices;
		uint32_t myCurrentIndex = 0;
		const uint32_t myStartIndex = 1;

		std::vector<GPUMaterial> myGPUMaterials;
		std::unordered_map<SubMaterial*, uint32_t> myMaterialIndexMap;
		Ref<ShaderStorageBufferSet> myStorageBuffer;
	};
}
