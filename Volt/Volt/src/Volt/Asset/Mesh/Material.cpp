#include "vtpch.h"
#include "Material.h"

#include "Volt/Asset/Mesh/SubMaterial.h"

namespace Volt
{
	Ref<SubMaterial> Material::CreateSubMaterial(Ref<Shader> shader)
	{
		const uint32_t index = (uint32_t)mySubMaterials.size();
		auto material = mySubMaterials.emplace(index, SubMaterial::Create("New Material", index, shader));
	
		return mySubMaterials.at(index);
	}
}

