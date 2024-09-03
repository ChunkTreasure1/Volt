#pragma once

#include "Volt/Asset/AssetTypes.h"

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetFactory.h>

namespace Volt
{
	class Mesh;
	class MeshSource : public Asset
	{
	public:
		MeshSource();
		~MeshSource() override = default;

		static AssetType GetStaticType() { return AssetTypes::MeshSource; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }
	
		inline Ref<Mesh> GetUnderlyingMesh() const { return m_underlyingMesh; }

	private:
		friend class SourceMeshSerializer;

		Ref<Mesh> m_underlyingMesh;
	};
}
