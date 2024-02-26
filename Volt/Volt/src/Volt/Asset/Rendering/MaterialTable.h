#pragma once

#include "Volt/Scene/Reflection/ComponentReflection.h"

#include <CoreUtilities/Core.h>

#include <vector>

namespace Volt
{
	class Material;

	class MaterialTable
	{
	public:
		void SetMaterial(AssetHandle material, uint32_t index);
		AssetHandle GetMaterial(uint32_t index) const;

		bool ContainsMaterialIndex(uint32_t index) const;

		inline const bool IsValid() const { return !m_materials.empty(); }
		inline const uint32_t GetSize() const { return static_cast<uint32_t>(m_materials.size()); }

		std::vector<AssetHandle>::iterator begin() { return m_materials.begin(); }
		std::vector<AssetHandle>::iterator end() { return m_materials.end(); }

		const std::vector<AssetHandle>::const_iterator begin() const { return m_materials.cbegin(); }
		const std::vector<AssetHandle>::const_iterator end() const { return m_materials.cend(); }
		
	private:
		std::vector<AssetHandle> m_materials;
	};
}
