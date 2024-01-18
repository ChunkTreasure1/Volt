#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Rendering/MaterialTable.h"

namespace Volt
{
	class Mesh;
	class MeshCompiler
	{
	public:
		static bool TryCompile(Ref<Mesh> mesh, const std::filesystem::path& destination, const MaterialTable& materialTable = MaterialTable());
		static size_t CalculateMeshSize(Ref<Mesh> mesh);

	private:
		static void SetupMaterials(Ref<Mesh> mesh, const std::filesystem::path& destination);
	};
}
