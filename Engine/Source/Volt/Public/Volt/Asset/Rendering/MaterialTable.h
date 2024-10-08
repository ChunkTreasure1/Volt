#pragma once
#include <AssetSystem/AssetHandle.h>
#include <CoreUtilities/Core.h>

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

		Vector<AssetHandle>::iterator begin() { return m_materials.begin(); }
		Vector<AssetHandle>::iterator end() { return m_materials.end(); }

		const Vector<AssetHandle>::const_iterator begin() const { return m_materials.cbegin(); }
		const Vector<AssetHandle>::const_iterator end() const { return m_materials.cend(); }
		
	private:
		Vector<AssetHandle> m_materials;
	};
}
