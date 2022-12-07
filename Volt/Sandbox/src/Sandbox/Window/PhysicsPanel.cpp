#include "sbpch.h"
#include "PhysicsPanel.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Physics/PhysicsLayer.h>
#include <Volt/Physics/Physics.h>

PhysicsPanel::PhysicsPanel()
	: EditorWindow("Physics Panel")
{}

void PhysicsPanel::UpdateMainContent()
{
	const ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;

	if (ImGui::BeginTable("settingsMain", 2, tableFlags))
	{
		ImGui::TableSetupColumn("Outline", 0, 250.f);
		ImGui::TableSetupColumn("View", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		DrawOutline();

		ImGui::TableNextColumn();

		DrawView();

		ImGui::EndTable();
	}
}

void PhysicsPanel::DrawOutline()
{
	ImGuiStyle& style = ImGui::GetStyle();
	auto color = style.Colors[ImGuiCol_FrameBg];
	UI::ScopedColor newColor(ImGuiCol_ChildBg, { color.x, color.y, color.z, color.w });

	ImGui::BeginChild("##outline");

	UI::ShiftCursor(5.f, 5.f);

	if (ImGui::Selectable("General"))
	{
		myCurrentMenu = Menu::General;
	}

	UI::ShiftCursor(5.f, 5.f);

	if (ImGui::Selectable("Layers"))
	{
		myCurrentMenu = Menu::Layers;
	}

	ImGui::EndChild();
}

void PhysicsPanel::DrawView()
{
	ImGui::BeginChild("##view", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight()));
	{
		ImGui::BeginChild("scrolling");

		switch (myCurrentMenu)
		{
			case Menu::General: DrawGeneralMenu(); break;
			case Menu::Layers: DrawLayersMenu(); break;
			default: break;
		}

		ImGui::EndChild();
	}
	ImGui::EndChild();
}

void PhysicsPanel::DrawGeneralMenu()
{
	UI::PushId();
	if (UI::BeginProperties(""))
	{
		UI::Property("Gravity", const_cast<gem::vec3&>(Volt::Physics::GetSettings().gravity));
		UI::Property("World Max Bounds", const_cast<gem::vec3&>(Volt::Physics::GetSettings().worldBoundsMax));
		UI::Property("World Min Bounds", const_cast<gem::vec3&>(Volt::Physics::GetSettings().worldBoundsMin));

		UI::Property("World Sub Divisions", const_cast<uint32_t&>(Volt::Physics::GetSettings().worldBoundsSubDivisions));
		UI::Property("Solver Iterations", const_cast<uint32_t&>(Volt::Physics::GetSettings().solverIterations));
		UI::Property("Solver Velocity Iterations", const_cast<uint32_t&>(Volt::Physics::GetSettings().solverIterations));

		{
			std::vector<const char*> boradphaseNames = { "Sweep And Prune", "Multi Box Prune", "Automatic Box Prune" };
			int32_t& broadphaseType = (int32_t&)const_cast<Volt::BroadphaseType&>(Volt::Physics::GetSettings().broadphaseAlgorithm);

			UI::ComboProperty("Broadphase Type", broadphaseType, boradphaseNames);
		}

		{
			std::vector<const char*> frictonNames = { "Patch", "One Directional", "Two Directional" };
			int32_t& frictionType = (int32_t&)const_cast<Volt::FrictionType&>(Volt::Physics::GetSettings().frictionModel);

			UI::ComboProperty("Friction Type", frictionType, frictonNames);
		}

		UI::EndProperties();
	}
	UI::PopId();
}

void PhysicsPanel::DrawLayersMenu()
{
	if (ImGui::Button("+"))
	{
		Volt::PhysicsLayerManager::AddLayer("New Layer");
	}

	int32_t layerToRemove = -1;

	for (auto& layer : Volt::PhysicsLayerManager::GetLayers())
	{
		const std::string layerId = layer.name + "###" + std::to_string(layer.layerId);
		if (ImGui::CollapsingHeader(layerId.c_str()))
		{
			UI::PushId();
			if (UI::BeginProperties(""))
			{
				UI::Property("Name", layer.name);
				for (auto& otherLayer : Volt::PhysicsLayerManager::GetLayers())
				{
					bool currVal = Volt::PhysicsLayerManager::ShouldCollide(layer.layerId, otherLayer.layerId);
					if (UI::Property(otherLayer.name, currVal))
					{
						Volt::PhysicsLayerManager::SetLayerCollision(layer.layerId, otherLayer.layerId, currVal);
					}
				}

				UI::EndProperties();
			}
			UI::PopId();
			
			{
				UI::ScopedColor color{ ImGuiCol_Button, { 0.2f, 0.2f, 0.2f, 1.f } };
				std::string buttonId = "Remove###rem" + std::to_string(layer.layerId);
				if (ImGui::Button(buttonId.c_str()))
				{
					layerToRemove = (int32_t)layer.layerId;
				}
			}
		}
	}

	if (layerToRemove > -1)
	{
		Volt::PhysicsLayerManager::RemoveLayer((uint32_t)layerToRemove);
	}
}