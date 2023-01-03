#include "sbpch.h"
#include "NavigationPanel.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/MeshCompiler.h"
#include "Volt/Components/Components.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Asset/AssetManager.h"

#include <imgui_stdlib.h>

void NavigationPanel::UpdateMainContent()
{
	if (auto& nvm = Volt::NavigationSystem::Get().GetNavMesh())
	{
		myVTNavMesh = nvm;
	}

	if (ImGui::BeginTabBar("tabs"))
	{
		if (ImGui::BeginTabItem("Bake"))
		{
			BakeTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Objects"))
		{
			ObjectsTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Debug"))
		{
			DebugTab();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	if (myVTNavMesh)
	{
		Volt::Renderer::Submit(myVTNavMesh, gem::mat4(1.f));
	}
}

bool NavigationPanel::CreateObjFile(Ref<Volt::Mesh> mesh, const std::string& path)
{
	std::ofstream file;
	file.open(path);
	if (!file.is_open()) { return false; };

	for (const auto& v : mesh->GetVertices())
	{
		file << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
	}
	for (const auto& v : mesh->GetVertices())
	{
		file << "vt " << v.texCoords[0].x << " " << v.texCoords[0].y << "\n";
	}
	for (const auto& v : mesh->GetVertices())
	{
		file << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << "\n";
	}
	auto indices = mesh->GetIndices();
	for (uint32_t i = 0; i < mesh->GetIndexCount(); i += 3)
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

	return true;
}

void NavigationPanel::BakeTab()
{
	ImGui::SliderFloat("Max Climb", &myBuilder.GetBuildSettings().agentMaxClimb, 0.1f, 5.f);

	static bool manual = false;
	static Volt::AssetHandle manualHandle;
	ImGui::Checkbox("Manual Mode", &manual);

	if (manual)
	{
		auto path = Volt::AssetManager::Get().GetPathFromAssetHandle(manualHandle).string();
		if (path.empty()) { path = "Drag .vtmesh"; }

		ImGui::InputTextString("##mnvm", &path, ImGuiInputTextFlags_ReadOnly);
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM");
			if (payload)
			{
				manualHandle = *(Volt::AssetHandle*)payload->Data;
			}
			ImGui::EndDragDropTarget();
		}
	}

	if (ImGui::Button("Bake NavMesh"))
	{
		auto scenePath = myCurrentScene->path;
		scenePath.remove_filename();

		if (!manual)
		{
			auto filePath = scenePath.string() + "NavMesh.obj";

			auto ctx = myBuilder.GetContext();
			if (!myGeometry.load(ctx, filePath))
			{
				VT_CORE_ERROR(ctx->getLogText(ctx->getLogCount() - 1));
			}
			else
			{
				VT_CORE_INFO(std::format("{0} loaded successfully!", filePath));
			}

			myBuilder.ChangeMesh(&myGeometry);
			myBuilder.BuildNavMesh(10.f);
		}
		else
		{
			myBuilder.BuildManualNavMesh(Volt::AssetManager::GetAsset<Volt::Mesh>(manualHandle));
		}

		auto navmeshPath = scenePath;
		navmeshPath.append("NavMesh.vtnavmesh");

		if (myVTNavMesh)
		{
			*myVTNavMesh = *myBuilder.GetNavMesh();
		}
		else
		{
			myVTNavMesh = myBuilder.GetNavMesh();
		}
		myVTNavMesh->path = navmeshPath;

		Volt::AssetManager::Get().SaveAsset(myVTNavMesh);
	}
}

void NavigationPanel::ObjectsTab()
{
	if (ImGui::Button("Create Obj"))
	{
		std::vector<Volt::Entity> walkableMeshes;

		myCurrentScene->GetRegistry().ForEach<Volt::MeshComponent>([&](Wire::EntityId id, Volt::MeshComponent& meshComp)
			{
				if (meshComp.walkable)
				{
					walkableMeshes.emplace_back(Volt::Entity(id, myCurrentScene.get()));
				}
			});

		auto navmesh = Volt::NavMeshConverter::CombineMeshes(walkableMeshes, 0.1f);
		if (navmesh) 
		{
			auto scenePath = myCurrentScene->path;
			scenePath.remove_filename();

			// Compile to obj for debugging
			CreateObjFile(navmesh, scenePath.string() + "NavMesh.obj");
		}
	}

	if (ImGui::Button("Export NavMesh"))
	{
		if (myVTNavMesh)
		{
			auto scenePath = myCurrentScene->path;
			scenePath.remove_filename();

			if (!scenePath.empty())
			{
				Volt::MeshCompiler::TryCompile(myVTNavMesh, scenePath.string() + "ExportedNavMesh.vtmesh", myVTNavMesh->GetMaterial()->handle);
			}
		}
	}
}

void NavigationPanel::DebugTab()
{
	static bool showPolygonEdges = false;
	static bool showPaths = false;
	static bool showPortals = false;
	static bool showPathfinder = false;

	if (ImGui::Button(std::format("{0} PolyEdges", (showPolygonEdges) ? "Hide" : "Show").c_str())) { showPolygonEdges = !showPolygonEdges; }
	if (ImGui::Button(std::format("{0} Paths", (showPaths) ? "Hide" : "Show").c_str())) { showPaths = !showPaths; }
	if (ImGui::Button(std::format("{0} Portals", (showPortals) ? "Hide" : "Show").c_str())) { showPortals = !showPortals; }
	ImGui::NewLine();
	if (ImGui::Button(std::format("{0} Pathfind", (showPathfinder) ? "Disable" : "Enable").c_str())) { showPathfinder = !showPathfinder; }

	// Debug NavMesh
	if (myVTNavMesh)
	{
		const auto& navMeshData = myVTNavMesh->GetNavMeshData();

		if (showPathfinder)
		{
			static Volt::Entity startEnt;
			static Volt::Entity endEnt;

			auto sName = (!startEnt.IsNull()) ? startEnt.GetComponent<Volt::TagComponent>().tag : "Drag entity";
			auto eName = (!endEnt.IsNull()) ? endEnt.GetComponent<Volt::TagComponent>().tag : "Drag entity";

			ImGui::InputTextString("##feid", &sName, ImGuiInputTextFlags_ReadOnly);
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy");
				if (payload)
				{
					startEnt = Volt::Entity(*(Wire::EntityId*)payload->Data, myCurrentScene.get());
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::InputTextString("##seid", &eName, ImGuiInputTextFlags_ReadOnly);
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy");
				if (payload)
				{
					endEnt = Volt::Entity(*(Wire::EntityId*)payload->Data, myCurrentScene.get());
				}
				ImGui::EndDragDropTarget();
			}

			if (!startEnt.IsNull() && !endEnt.IsNull())
			{
				static std::vector<Pathfinder::pfLink> usedPortals;
				static auto path = navMeshData.findPath(Volt::VTtoPF(startEnt.GetPosition()), Volt::VTtoPF(endEnt.GetPosition()), &usedPortals);

				if (ImGui::Button("Find Path"))
				{
					path = navMeshData.findPath(Volt::VTtoPF(startEnt.GetPosition()), Volt::VTtoPF(endEnt.GetPosition()), &usedPortals);
				}

				static int debugPortalIndex = 0;
				ImGui::SliderInt("Portal Index", &debugPortalIndex, 0, (usedPortals.empty()) ? 0 : usedPortals.size() - 1);

				auto nvd = navMeshData;
				for (uint32_t i = 0; !path.empty() && i < path.size() - 1; i++)
				{
					auto current = path[i];
					auto next = path[i + 1];

					gem::vec3 start = { current.x, current.y + 10.f, current.z };
					gem::vec3 end = { next.x, next.y + 10.f, next.z };

					Volt::Renderer::SubmitLine(start, end, gem::vec4(0.f, 0.f, 0.f, 1.f));
				}

				if (!path.empty() && !usedPortals.empty())
				{
					gem::vec3 apex = { path.back().x, path.back().y, path.back().z };
					for (uint32_t i = 0; i < usedPortals.size(); i++)
					{
						const auto& portal_edge = navMeshData.getLinkEdge(usedPortals[i]);

						gem::vec3 start = { portal_edge.start.x, portal_edge.start.y + 15.f, portal_edge.start.z };
						gem::vec3 end = { portal_edge.end.x, portal_edge.end.y + 15.f, portal_edge.end.z };

						Volt::Renderer::SubmitLine(start, end, gem::vec4(1.f, 1.f, 1.f, 1.f));

						for (const auto& p : path)
						{
							if (auto distStart = gem::distance({ p.x, start.y, p.z }, start); i < debugPortalIndex && distStart < 0.01f)
							{
								apex = start;
							}
							if (auto distEnd = gem::distance({ p.x, end.y, p.z }, end); i < debugPortalIndex && distEnd < 0.01f)
							{
								apex = end;
							}
						}
					}

					// Debug specific portal
					{
						const auto& portal_edge = navMeshData.getLinkEdge(usedPortals[debugPortalIndex]);

						gem::vec3 start = { portal_edge.start.x, portal_edge.start.y + 15.f, portal_edge.start.z };
						gem::vec3 end = { portal_edge.end.x, portal_edge.end.y + 15.f, portal_edge.end.z };

						Volt::Renderer::SubmitLine(apex, start, gem::vec4(1.f, 0.f, 0.f, 1.f));
						Volt::Renderer::SubmitLine(apex, end, gem::vec4(0.f, 0.f, 1.f, 1.f));
					}
				}
			}
		}

		if (showPolygonEdges)
		{
			for (uint32_t i = 0; const auto & edge : navMeshData.getEdges())
			{
				gem::vec3 start = { edge.start.x, edge.start.y + 5.f, edge.start.z };
				gem::vec3 end = { edge.end.x, edge.end.y + 5.f, edge.end.z };

				Volt::Renderer::SubmitLine(start, end, gem::vec4(1.f, 1.f, 1.f, 1.f));
			}
		}

		if (showPaths)
		{
			for (uint32_t i = 0; i < navMeshData.getPolyCount(); i++)
			{
				auto center = navMeshData.getCenter(i);

				gem::vec3 start = { center.x, center.y + 10.f, center.z };

				for (const auto& portal : navMeshData.getLinks(i))
				{
					auto nextCenter = navMeshData.getLinkEdge(portal).GetCenter();
					gem::vec3 end = { nextCenter.x, nextCenter.y + 10.f, nextCenter.z };
					Volt::Renderer::SubmitLine(start, end, gem::vec4(1.f, 0.f, 0.f, 1.f));
				}
			}
		}

		if (showPortals)
		{
			for (const auto& portal : navMeshData.getLinks())
			{
				auto portal_edge = myVTNavMesh->GetNavMeshData().getLinkEdge(portal);
				gem::vec3 start = { portal_edge.start.x, portal_edge.start.y + 15.f, portal_edge.start.z };
				gem::vec3 end = { portal_edge.end.x, portal_edge.end.y + 15.f, portal_edge.end.z };

				Volt::Renderer::SubmitLine(start, end, gem::vec4(0.f, 0.f, 1.f, 1.f));
			}
		}
	}
}
