#include "vtpch.h"
#include "Material.h"

#include "Volt/Asset/Mesh/SubMaterial.h"

namespace Volt
{
	Ref<SubMaterial> Material::GetSubMaterialAt(const uint32_t index) const
	{
		if (!mySubMaterials.contains(index))
		{
			return nullptr;
		}

		return mySubMaterials.at(index);
	}

	Ref<SubMaterial> Material::CreateSubMaterial(Ref<Shader> shader)
	{
		const uint32_t index = (uint32_t)mySubMaterials.size();
		auto material = mySubMaterials.emplace(index, SubMaterial::Create("New Material", index, shader));
	
		return mySubMaterials.at(index);
	}

	Ref<Material> Material::Create(Ref<Shader> shader, const uint32_t subMaterialCount)
	{
		auto mat = CreateRef<Material>();
		mat->CreateSubMaterial(shader);
		
		return mat;
	}
}

