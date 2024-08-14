#include "vtpch.h"
#include "MeshSource.h"

#include "Volt/Asset/Mesh/Mesh.h"

namespace Volt
{
	VT_REGISTER_ASSET_FACTORY(AssetTypes::MeshSource, MeshSource);

	MeshSource::MeshSource()
	{
		m_underlyingMesh = CreateRef<Mesh>();
	}
}
