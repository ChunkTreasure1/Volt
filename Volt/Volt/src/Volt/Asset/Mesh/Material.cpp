 #include "vtpch.h"
#include "Material.h"

#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	Material::Material(const Material& material)
	{
		myName = material.myName;

		for (const auto& [index, subMaterial] : material.mySubMaterials)
		{
			mySubMaterials[index] = CreateRef<SubMaterial>(*subMaterial);
		}
	}

	Ref<SubMaterial> Material::GetSubMaterialAt(const uint32_t index) const
	{
		if (!mySubMaterials.contains(index))
		{
			return nullptr;
		}

		return mySubMaterials.at(index);
	}

	Ref<SubMaterial> Material::TryGetSubMaterialAt(const uint32_t index) const
	{
		if (!mySubMaterials.contains(index))
		{
			if (!mySubMaterials.contains(0))
			{
				return Renderer::GetDefaultData().defaultMaterial;
			}

			return mySubMaterials.at(0);
		}

		return mySubMaterials.at(index);
	}

	Ref<SubMaterial> Material::CreateSubMaterial(Ref<Shader> shader, const std::string& name)
	{
		const uint32_t index = (uint32_t)mySubMaterials.size();
		auto material = mySubMaterials.emplace(index, SubMaterial::Create(name, index, shader));

		return mySubMaterials.at(index);
	}

	Ref<SubMaterial> Material::CreateSubMaterial(const RenderPipelineSpecification& specification)
	{
		const uint32_t index = (uint32_t)mySubMaterials.size();
		auto material = mySubMaterials.emplace(index, SubMaterial::Create("New Material", index, specification));

		return mySubMaterials.at(index);
	}

	Ref<Material> Material::Create(Ref<Shader> shader, const uint32_t subMaterialCount)
	{
		auto mat = CreateRef<Material>();

		for (uint32_t i = 0; i < subMaterialCount; i++)
		{
			mat->CreateSubMaterial(shader);
		}

		return mat;
	}

	Ref<Material> Material::Create(const RenderPipelineSpecification& specification, const uint32_t subMaterialCount)
	{
		auto mat = CreateRef<Material>();

		for (uint32_t i = 0; i < subMaterialCount; i++)
		{
			mat->CreateSubMaterial(specification);
		}

		return mat;
	}
}

