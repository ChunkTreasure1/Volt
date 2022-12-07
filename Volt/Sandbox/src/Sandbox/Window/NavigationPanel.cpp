#include "sbpch.h"
#include "NavigationPanel.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Components/Components.h"
#include "Volt/AI/NMConverter.h"

void NavigationPanel::UpdateMainContent()
{
	ImGui::Text("NavMesh creates obj file.");
	ImGui::Text("The obj file needs to be manually corrected to work.");
	if (ImGui::Button("Create NavMesh.obj"))
	{
		std::vector<Volt::Entity> walkableMeshes;

		myCurrentScene->GetRegistry().ForEach<Volt::MeshComponent>([&](Wire::EntityId id, Volt::MeshComponent& meshComp)
			{
				if (meshComp.walkable)
				{
					walkableMeshes.emplace_back(Volt::Entity(id, myCurrentScene.get()));
				}
			});

		auto navmesh = Volt::NMConverter::CombineMeshes(walkableMeshes, 0.1f);
		if (!navmesh) { return; }

		// Compile to obj for debugging
		{
			auto scenePath = myCurrentScene->path;
			scenePath.remove_filename();

			std::ofstream file;
			file.open(scenePath.string() + "NavMesh.obj");

			for (const auto& v : navmesh->GetVertices())
			{
				file << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
			}
			for (const auto& v : navmesh->GetVertices())
			{
				file << "vt " << v.texCoords[0].x << " " << v.texCoords[0].y << "\n";
			}
			for (const auto& v : navmesh->GetVertices())
			{
				file << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << "\n";
			}
			auto indices = navmesh->GetIndices();
			for (uint32_t i = 0; i < navmesh->GetIndexCount(); i += 3)
			{
				auto i1 = indices[i] + 1;
				auto i2 = indices[i + 1] + 1;
				auto i3 = indices[i + 2] + 1;

				file << "f "
					<< i1 << "/" << i1 << "/" << i1 << " "
					<< i2 << "/" << i2 << "/" << i2 << " "
					<< i3 << "/" << i3 << "/" << i3 << "\n";
			}
			file.close();
		}
	}

	if (ImGui::Button("Crash Me"))
	{
		Ref<float> x;
		*x = 0.f;
	}
}
