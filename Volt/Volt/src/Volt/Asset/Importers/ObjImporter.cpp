#include "vtpch.h"
#include "ObjImporter.h"

#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	void ObjImporter::ExportMeshImpl(std::vector<Ref<Mesh>> assets, const std::filesystem::path& path)
	{
		std::ofstream file;
		file.open(Volt::AssetManager::GetContextPath(path) / path);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Could not open mesh file!");
			return;
		};

		std::stringstream SSvertices;
		std::stringstream SStexCoords;
		std::stringstream SSnormals;
		std::stringstream SSindices;

		uint32_t indexOffset = 0;

		for (const auto& mesh : assets)
		{
			for (const auto& v : mesh->GetVertices())
			{
				SSvertices << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
			}
			for (const auto& v : mesh->GetVertices())
			{
				SStexCoords << "vt " << v.texCoords.x << " " << v.texCoords.y << "\n";
			}
			for (const auto& v : mesh->GetVertices())
			{
				SSnormals << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << "\n";
			}

			auto indices = mesh->GetIndices();
			for (auto& ind : indices)
			{
				ind += indexOffset;
			}

			for (uint32_t i = 0; i < mesh->GetIndexCount(); i += 3)
			{
				auto i1 = indices[i] + 1;
				auto i2 = indices[i + 1] + 1;
				auto i3 = indices[i + 2] + 1;

				SSindices << "f "
					<< i1 << "/" << i1 << "/" << i1 << " "
					<< i2 << "/" << i2 << "/" << i2 << " "
					<< i3 << "/" << i3 << "/" << i3 << "\n";
			}

			indexOffset += (uint32_t)indices.size();
		}

		file << SSvertices.str() << SStexCoords.str() << SSnormals.str() << SSindices.str();

		file.close();

		return;
	}
}
