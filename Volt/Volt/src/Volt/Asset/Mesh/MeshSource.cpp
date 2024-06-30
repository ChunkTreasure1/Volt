#include "vtpch.h"
#include "MeshSource.h"

#include "Volt/Asset/Mesh/Mesh.h"

namespace Volt
{
	MeshSource::MeshSource()
	{
		m_underlyingMesh = CreateRef<Mesh>();
	}
}
