#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class SubMaterial;
	class Shader;
	class Material : public Asset
	{
	public:
		Material() = default;

		Ref<SubMaterial> GetSubMaterialAt(const uint32_t index) const;

		inline const std::unordered_map<uint32_t, Ref<SubMaterial>>& GetSubMaterials() const { return mySubMaterials; }
		inline const uint32_t GetSubMaterialCount() const { return (uint32_t)mySubMaterials.size(); }

		inline const std::string& GetName() const { return myName; }
		inline void SetName(const std::string& aName) { myName = aName; }

		Ref<SubMaterial> CreateSubMaterial(Ref<Shader> shader);

		static AssetType GetStaticType() { return AssetType::Material; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Material> Create(Ref<Shader> shader, const uint32_t subMaterialCount = 1);

	private:
		friend class FbxImporter;
		friend class VTMeshImporter;
		friend class MaterialImporter;

		std::string myName;
		std::unordered_map<uint32_t, Ref<SubMaterial>> mySubMaterials;
	};
}