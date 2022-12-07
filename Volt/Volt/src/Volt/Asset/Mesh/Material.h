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

		inline const std::unordered_map<uint32_t, Ref<SubMaterial>>& GetSubMaterials() const { return mySubMaterials; }
		inline const std::string& GetName() const { return myName; }
		inline void SetName(const std::string& aName) { myName = aName; }

		Ref<SubMaterial> CreateSubMaterial(Ref<Shader> shader);

		static AssetType GetStaticType() { return AssetType::Material; }
		AssetType GetType() override { return GetStaticType(); }

	private:
		friend class FbxImporter;
		friend class VTMeshImporter;
		friend class MaterialImporter;

		std::string myName;
		std::unordered_map<uint32_t, Ref<SubMaterial>> mySubMaterials;
	};
}