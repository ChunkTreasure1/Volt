#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class Mesh;
	class MeshCompiler
	{
	public:
		static bool TryCompile(Ref<Mesh> mesh, const std::filesystem::path& destination, AssetHandle materialHandle = Asset::Null());

	private:
		static size_t CalculateMeshSize(Ref<Mesh> mesh);
		static void CreateMaterial(Ref<Mesh> mesh, const std::filesystem::path& destination);
	};
}