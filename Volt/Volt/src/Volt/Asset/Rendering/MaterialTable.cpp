#include "vtpch.h"
#include "MaterialTable.h"

namespace Volt
{
	void MaterialTable::SetMaterial(AssetHandle material, uint32_t index)
	{
		if (static_cast<size_t>(index) >= m_materials.size())
		{
			m_materials.resize(index + 1);
		}

		m_materials[index] = material;
	}

	AssetHandle MaterialTable::GetMaterial(uint32_t index) const
	{
		VT_CORE_ASSERT(static_cast<size_t>(index) < m_materials.size(), "Trying to access invalid material!");
		return m_materials.at(index);
	}

	bool MaterialTable::ContainsMaterialIndex(uint32_t index) const
	{
		return static_cast<size_t>(index) >= m_materials.size();
	}
}
