#include "sbpch.h"
#include "VertexPainterPanel.h"

#include <Volt/Input/Input.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Rendering/DebugRenderer.h>

#include <Volt/Components/CoreComponents.h>
#include <Volt/Components/RenderingComponents.h>

#include <Volt/Rendering/Camera/Camera.h>

#include <Volt/Events/Event.h>
#include <Volt/Core/Base.h>
#include "Sandbox/Utility/SelectionManager.h"

#include <Volt/Utility/UIUtility.h>
#include "Sandbox/Utility/EditorResources.h"

#include <Volt/Input/KeyCodes.h>
#include <Volt/Scene/SceneManager.h>
#include <Volt/Math/RayTriangle.h>
#include <Volt/Utility/PackUtility.h>

#include <Sandbox/Utility/EditorLibrary.h>
#include <Sandbox/Window/ViewportPanel.h>

VertexPainterPanel::VertexPainterPanel(Ref<Volt::Scene>& in_scene, Ref<EditorCameraController>& in_cc)
	: ex_scene(in_scene), ex_cameraController(in_cc), EditorWindow("Vertex Painting")
{
}

VertexPainterPanel::~VertexPainterPanel()
{
}

void VertexPainterPanel::UpdateMainContent()
{
	PanelDraw();
	BillboardDraw();

	BrushUpdate();
	if (Volt::Input::IsMouseButtonDown(0) && Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT)) Paint(m_settings.paintColor);
	else if (Volt::Input::IsMouseButtonDown(0) && Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL)) Paint(m_settings.eraseColor);

	Volt::DebugRenderer::DrawLine(ray.pos + ray.dir * 10.f, m_brushPosition, { 1,0,0,1 });
	Volt::DebugRenderer::DrawBillboard(m_brushPosition, { 0.1f,0.1f,0.1f }, { 0,1,0,1 });
}

bool VertexPainterPanel::OnViewportResizeEvent(Volt::ViewportResizeEvent& e)
{
	myViewportSize = { e.GetWidth(), e.GetHeight() };
	myViewportPosition = { e.GetX(), e.GetY() };
	return false;
}

void VertexPainterPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(VertexPainterPanel::UpdateDeltaTime));
	dispatcher.Dispatch<Volt::ViewportResizeEvent>(VT_BIND_EVENT_FN(VertexPainterPanel::OnViewportResizeEvent));
}

bool VertexPainterPanel::BrushUpdate()
{
	//if (!m_settings.isSelecting)
	{
		VT_PROFILE_SCOPE("Vertex Painter construct and place mesh");

		auto viewportPanel = EditorLibrary::Get<ViewportPanel>();
		auto rayDir = ex_cameraController->GetCamera()->ScreenToWorldRay(viewportPanel->GetViewportLocalPosition(ImGui::GetMousePos()), viewportPanel->GetSize());

		ray.dir = rayDir;
		ray.pos = ex_cameraController->GetCamera()->GetPosition();

		std::vector<glm::vec3> intersectionPoints;
		glm::vec3 intersectionPoint;
		for (auto wireId : SelectionManager::GetSelectedEntities())
		{
			Volt::Entity currentEntity = ex_scene->GetEntityFromUUID(wireId);
			if (!currentEntity.HasComponent<Volt::MeshComponent>()) continue;

			auto meshComponent = currentEntity.GetComponent<Volt::MeshComponent>();
			auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(meshComponent.handle);
			//auto origin = ex_cameraController->GetCamera()->GetPosition() - currentEntity.GetPosition();
			auto origin = ex_cameraController->GetCamera()->GetPosition();
			auto localRayDir = rayDir;

			auto vList = mesh->GetVertices();
			auto iList = mesh->GetIndices();

			auto entTransform = currentEntity.GetTransform();

			for (auto& submesh : mesh->GetSubMeshes())
			{
				for (uint32_t index = submesh.vertexStartOffset; index < submesh.vertexStartOffset + submesh.vertexCount; index++)
				{
					vList[index].position = entTransform * submesh.transform * glm::vec4(vList[index].position, 1);
				}
			}

			for (int i = 0; i < iList.size(); i += 3)
			{
				auto p1 = vList[iList[i]].position;
				auto p2 = vList[iList[i + 1]].position;
				auto p3 = vList[iList[i + 2]].position;

				intersectionPoint = { 0,0,0 };
				if (Volt::rayTriangleIntersection(origin, localRayDir, p1, p2, p3, intersectionPoint))
				{
					if (std::isnan(intersectionPoint.x) ||
						std::isnan(intersectionPoint.y) ||
						std::isnan(intersectionPoint.z) ||
						std::isinf(intersectionPoint.x) ||
						std::isinf(intersectionPoint.y) ||
						std::isinf(intersectionPoint.z))
						continue;
					intersectionPoints.push_back(intersectionPoint);
				}
			}
		}

		if (intersectionPoints.empty())
		{
			m_brushPosition = { 1000000, 0, 0 };
			return false;
		}

		auto iPoint = intersectionPoints[0];
		auto cPos = ex_cameraController->GetCamera()->GetPosition();
		for (const auto& point : intersectionPoints)
		{
			if (glm::distance(iPoint, cPos) > glm::distance(point, cPos)) iPoint = point;
		}
		//VT_INFO(std::to_string(iPoint.x) + " : " + std::to_string(iPoint.y) + " : " + std::to_string(iPoint.z));
		m_brushPosition = iPoint;
	}
	return false;
}

void VertexPainterPanel::PanelDraw()
{
	// Toolbar begin
	{
		Ref<Volt::Texture2D> paintIcon = EditorResources::GetEditorIcon(EditorIcon::Paint);
		Ref<Volt::Texture2D> selectIcon = EditorResources::GetEditorIcon(EditorIcon::Select);
		Ref<Volt::Texture2D> fillIcon = EditorResources::GetEditorIcon(EditorIcon::Fill);
		Ref<Volt::Texture2D> swapIcon = EditorResources::GetEditorIcon(EditorIcon::Swap);
		Ref<Volt::Texture2D> removeIcon = EditorResources::GetEditorIcon(EditorIcon::Remove);
		Ref<Volt::Texture2D> viewIcon = EditorResources::GetEditorIcon(EditorIcon::Visible);
		ImVec4 tintC = { 1,1,1,1 };
		ImVec4 selectBgc = { 0,0,0,0 };
		ImVec4 paintBgc = { 0,0,0,0 };
		ImVec4 viewColor = { 0,0,0,0 };
		ImVec2 buttonSize = { 30,30 };

		if (m_settings.isSelecting)
		{
			selectBgc = { 0.1f, 0.25f, 0.8f, 1.f };
		}
		else
		{
			paintBgc = { 0.1f, 0.25f, 0.8f, 1.f };
		}

		if (m_settings.viewEnabled)
		{
			viewColor = { 0.1f, 0.25f, 0.8f, 1.f };
		}

		if (UI::ImageButton("##vpSelectionMode", UI::GetTextureID(selectIcon), buttonSize, selectBgc, tintC))
		{
			SetView(false);
			m_settings.isSelecting = true;
			SelectionManager::Unlock();
		}
		ImGui::SameLine();
		if (UI::ImageButton("##vpPaintMode", UI::GetTextureID(paintIcon), buttonSize, paintBgc, tintC))
		{
			m_settings.isSelecting = false;
			SelectionManager::Lock();
		}
		ImGui::SameLine();
		if (UI::ImageButton("##vpFill", UI::GetTextureID(fillIcon), buttonSize))
		{
			for (const auto& _entId : SelectionManager::GetSelectedEntities())
			{
				auto ent = Volt::SceneManager::GetActiveScene()->GetEntityFromUUID(_entId);
				if (!AddPainted(ent)) continue;
				for (auto& vertex : ent.GetComponent<Volt::VertexPaintedComponent>().vertexColors)
				{
					glm::vec4 color =
					{
						m_settings.paintRedChannel ? m_settings.paintColor : 0,
							m_settings.paintGreenChannel ? m_settings.paintColor : 0,
							m_settings.paintBlueChannel ? m_settings.paintColor : 0,
							m_settings.paintAlphaChannel ? m_settings.paintColor : 0
					};

					const uint32_t packedColor = Volt::Utility::PackUNormFloat4AsUInt(color);
					vertex = packedColor;
				}
			}
		}
		ImGui::SameLine();
		if (UI::ImageButton("##vpSwap", UI::GetTextureID(swapIcon), buttonSize))
		{
			auto tempColor = m_settings.paintColor;
			m_settings.paintColor = m_settings.eraseColor;
			m_settings.eraseColor = tempColor;
		}
		ImGui::SameLine();
		if (UI::ImageButton("##vpRemove", UI::GetTextureID(removeIcon), buttonSize))
		{

			// Remove painted component
			for (const auto& _entId : SelectionManager::GetSelectedEntities())
			{
				auto ent = Volt::SceneManager::GetActiveScene()->GetEntityFromUUID(_entId);
				if (!ent.HasComponent<Volt::VertexPaintedComponent>()) continue;
				ent.RemoveComponent<Volt::VertexPaintedComponent>();
			}
		}
		ImGui::SameLine();
		if (UI::ImageButton("##vpView", UI::GetTextureID(viewIcon), buttonSize, viewColor, tintC))
		{
			SetView(!m_settings.viewEnabled);
		}

		// Tooltips
		auto hoveredItemId = ImGui::GetHoveredID();
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("##vpRemove"))
		{
			ImGui::SetTooltip("Remove paint from selection");
		}
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("##vpSwap"))
		{
			ImGui::SetTooltip("Swap paint and erase colors");
		}
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("##vpSelectionMode"))
		{
			ImGui::SetTooltip("Select mesh");
		}
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("##vpPaintMode"))
		{
			ImGui::SetTooltip("Paint mesh");
		}
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("##vpFill"))
		{
			ImGui::SetTooltip("Fill selection with paint color");
		}
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("##vpView"))
		{
			ImGui::SetTooltip("Show color blending view");
		}
		ImGui::Separator();
	}
	// Toolbar end

	// View begin
	ImGui::BeginChild("##properties", ImGui::GetContentRegionAvail(), false);
	if (ImGui::BeginTable("##vpVertexViewTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Properties", 0, 50);

		ImGui::TableNextColumn();

		ImGui::Dummy({ (ImGui::GetColumnWidth() * 0.5f) - (ImGui::CalcTextSize("General Settings   ").x * 0.5f),20 });
		ImGui::SameLine();
		ImGui::TextUnformatted("General Settings");
		ImGui::Separator();
		static std::vector<std::string> views = { "Red", "Green", "Blue", "Alpha", "All" };
		static int selectedView = 4;
		if (UI::Combo("View Channel", selectedView, views, ImGui::GetContentRegionAvail().x - 99))
		{
			m_settings.view = (Settings::eView)selectedView;
		}

		if (UI::BeginProperties("Vertex View Settings"))
		{
			UI::Property("Brush Size", m_settings.billboardRange, 0, 0, "Range from brush that Vertices are drawn");
			UI::PropertyDragFloat("Vertex Scalar", m_settings.billboadScalar, 0.1f, 0.1f, 10, "Vertex size");
			UI::PropertyDragFloat("Vertex Alpha", m_settings.billboardAlpha, 0.1f, 0.0f, 1.0f, "Vertex alpha outside of painting area");

			UI::EndProperties();
		}

		/*if (ImGui::Button("Default##vpResetView", { ImGui::GetContentRegionAvail().x, 0 }))
		{
			m_settings = Settings();
			SelectionManager::Unlock();
		}*/

		ImGui::TableNextColumn();
		ImGui::Dummy({ (ImGui::GetColumnWidth() * 0.5f) - (ImGui::CalcTextSize("Channels     ").x * 0.5f),20 });
		ImGui::SameLine();
		ImGui::TextUnformatted("Channels");
		ImGui::Separator();
		if (UI::BeginProperties("Color Painting Channels"))
		{
			UI::Property("Red", m_settings.paintRedChannel);
			UI::Property("Green", m_settings.paintGreenChannel);
			UI::Property("Blue", m_settings.paintBlueChannel);
			UI::Property("Alpha", m_settings.paintAlphaChannel);

			UI::EndProperties();
		}

		ImGui::EndTable();
	}
	// View end

	if (ImGui::CollapsingHeader("Painting"))
	{
		if (UI::BeginProperties("Brush Settings"))
		{
			//UI::Property("Brush Size", m_settings.brushRadius);
			UI::Property("Ignore Intencity", m_settings.ignoreIntencity);
			UI::PropertyDragFloat("Intensity", m_settings.intensity, 0.1f, 0, 0, "Value increment step per second");
			UI::PropertyDragFloat("Paint Color", m_settings.paintColor, 0.05f, 0, 1, "Target channel color when painting (SHIFT)");
			UI::PropertyDragFloat("Erase Color", m_settings.eraseColor, 0.05f, 0, 1, "Target channel color when erasing (CTRL)");

			UI::EndProperties();
		}
	}

	if (ImGui::CollapsingHeader("Material Management"))
	{
		//if (ImGui::TreeNode("Single Material"))
		//{		
		if (UI::BeginProperties("SingleMat"))
		{
			EditorUtils::Property("Material", m_materialSettings.singleMat, Volt::AssetType::Material);
			UI::EndProperties();
		}
		if (ImGui::Button("Apply To Selection##vpApplySingleMaterial", { ImGui::GetContentRegionAvail().x, 0 }))
		{
			for (auto entId : SelectionManager::GetSelectedEntities())
			{
				Volt::Entity entity = ex_scene->GetEntityFromUUID(entId);
				if (!entity.HasComponent<Volt::MeshComponent>()) continue;
				auto& meshComp = entity.GetComponent<Volt::MeshComponent>();
				meshComp.material = m_materialSettings.singleMat;
			}
		}
		/*ImGui::TreePop();
		}

		if (ImGui::TreeNode("Convert Material"))
		{
			if (ImGui::Button("Apply##vpApplyMaterialConversion", { ImGui::GetContentRegionAvail().x * 0.5f, 0 }))
			{

			}
			ImGui::SameLine();
			if (ImGui::Button("Revert##vpRevertMaterialConversion", { ImGui::GetContentRegionAvail().x * 0.5f, 0 }))
			{

			}
			if (UI::BeginProperties("SingleMat"))
			{
				EditorUtils::Property("Material", m_materialSettings.singleMat, Volt::AssetType::Material);
				UI::EndProperties();
			}
			ImGui::TreePop();
		}*/
	}
	ImGui::EndChild();
}

void VertexPainterPanel::BillboardDraw()
{
	for (auto id : SelectionManager::GetSelectedEntities())
	{
		auto paintedEnt = ex_scene->GetEntityFromUUID(id);
		if (paintedEnt.HasComponent<Volt::MeshComponent>())
		{
			auto meshComp = paintedEnt.GetComponent<Volt::MeshComponent>();
			auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(meshComp.handle);

			if (!mesh || !mesh->IsValid()) continue;
			bool hasPainted = paintedEnt.HasComponent<Volt::VertexPaintedComponent>();

			if (hasPainted)
			{
				auto vpMeshHandle = paintedEnt.GetComponent<Volt::VertexPaintedComponent>().meshHandle;
				auto vpasdjkghasdhjasjkld = paintedEnt.GetComponent<Volt::VertexPaintedComponent>();
				if (meshComp.handle != vpMeshHandle)
				{
					paintedEnt.RemoveComponent<Volt::VertexPaintedComponent>();
					continue;
				}
			}

			for (auto& submesh : mesh->GetSubMeshes())
			{
				for (uint32_t index = submesh.vertexStartOffset; index < submesh.vertexStartOffset + submesh.vertexCount; index++)
				{
					auto& vertex = mesh->GetVertices().at(index);

					glm::vec4 vertexColor = hasPainted ? Volt::Utility::UnpackUIntToUNormFloat4(paintedEnt.GetComponent<Volt::VertexPaintedComponent>().vertexColors[index]) : 0.f;

					auto vPos = glm::vec3(paintedEnt.GetTransform() * submesh.transform * glm::vec4(vertex.position, 1));
					if (!m_settings.isSelecting && glm::distance2(vPos, m_brushPosition) < m_settings.billboardRange * m_settings.billboardRange)
					{
						switch (m_settings.view)
						{
							case Settings::eView::RED:
								if (Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT))
									Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { m_settings.paintColor, 0, 0, 1 });
								else if (Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
									Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { m_settings.eraseColor, 0, 0, 1 });
								else
									Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { vertexColor.x, 0, 0, 1 });
								break;
							case Settings::eView::GREEN:
								if (Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT))
									Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { 1, m_settings.paintColor, 0, 1 });
								else if (Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
									Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { 1, m_settings.eraseColor, 0, 1 });
								else
									Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { 0, vertexColor.y, 0, 1 });
								break;
							case Settings::eView::BLUE:
								if (Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT))
									Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { 1, 1, m_settings.paintColor, 1 });
								else if (Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
									Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { 1, 1, m_settings.eraseColor,1 });
								else
									Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { 0, 0, vertexColor.z, 1 });

								break;
							case Settings::eView::ALPHA:
							{
								glm::vec4 alphaColor = { vertexColor.w, vertexColor.w, vertexColor.w, 1 };
								if (Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT))
									alphaColor = { m_settings.paintColor, m_settings.paintColor, m_settings.paintColor,1 };
								else if (Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
									alphaColor = { m_settings.eraseColor, m_settings.eraseColor, m_settings.eraseColor,1 };

								Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, alphaColor);
							} break;
							case Settings::eView::ALL:
							{

								glm::vec4 drawColor;

								if (Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT))
								{
									drawColor =
									{
										m_settings.paintRedChannel ? m_settings.paintColor : 0,
										m_settings.paintGreenChannel ? m_settings.paintColor : 0,
										m_settings.paintBlueChannel ? m_settings.paintColor : 0,
										1
									};
								}
								else if (Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
								{
									drawColor =
									{
										m_settings.paintRedChannel ? m_settings.eraseColor : 0,
										m_settings.paintGreenChannel ? m_settings.eraseColor : 0,
										m_settings.paintBlueChannel ? m_settings.eraseColor : 0,
										1
									};
								}
								else
								{
									drawColor = { vertexColor.x,vertexColor.y,vertexColor.z,1 };
								}
								Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, drawColor);

								break;
							}
							default:
								break;
						}
					}
					else
					{
						switch (m_settings.view)
						{
							case Settings::eView::RED:
								Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { vertexColor.x, 0, 0, m_settings.billboardAlpha });
								break;
							case Settings::eView::GREEN:
								Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { 0, vertexColor.y, 0, m_settings.billboardAlpha });
								break;
							case Settings::eView::BLUE:
								Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { 0, 0, vertexColor.z, m_settings.billboardAlpha });
								break;
							case Settings::eView::ALPHA:
							{
								// #mmax: broken, it no workie, do later, maybe
								glm::vec4 alphaColor = { vertexColor.w, vertexColor.w, vertexColor.w, m_settings.billboardAlpha };
								Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, alphaColor);
							} break;
							case Settings::eView::ALL:
								Volt::DebugRenderer::DrawBillboard(vPos, { .1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar,.1f * m_settings.billboadScalar }, { vertexColor.x, vertexColor.y, vertexColor.z, m_settings.billboardAlpha });
								break;
							default:
								break;
						}
					}
				}
			}
		}
	}
}

void VertexPainterPanel::CameraDraw()
{


}

bool VertexPainterPanel::AddPainted(Volt::Entity entity)
{
	if (!entity.HasComponent<Volt::MeshComponent>()) return false;
	if (entity.HasComponent<Volt::VertexPaintedComponent>()) return true;

	auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(entity.GetComponent<Volt::MeshComponent>().handle);
	auto& vpComp = entity.AddComponent<Volt::VertexPaintedComponent>();

	vpComp.vertexColors = std::vector<uint32_t>(mesh->GetVertices().size(), Volt::Utility::PackUNormFloat4AsUInt({ 0.f, 0.f, 0.f, 1.f }));
	vpComp.meshHandle = mesh->handle;

	//for (auto& vertex : entity.GetComponent<Volt::VertexPaintedComponent>().vertecies)
	//{
	//	vertex.color[0] = { 0,0,0,0 };
	//}

	return true;
}

void VertexPainterPanel::Paint(float color)
{
	if (m_settings.isSelecting) return;
	for (auto id : SelectionManager::GetSelectedEntities())
	{
		auto paintedEnt = ex_scene->GetEntityFromUUID(id);
		if (!paintedEnt.HasComponent<Volt::MeshComponent>()) continue;

		auto meshComp = paintedEnt.GetComponent<Volt::MeshComponent>();
		auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(meshComp.handle);

		if (!mesh || !mesh->IsValid()) continue;

		if (paintedEnt.HasComponent<Volt::VertexPaintedComponent>() && meshComp.handle != paintedEnt.GetComponent<Volt::VertexPaintedComponent>().meshHandle)
		{
			paintedEnt.RemoveComponent<Volt::VertexPaintedComponent>();
		}

		if (!AddPainted(paintedEnt)) continue;
		auto& vertecies = paintedEnt.GetComponent<Volt::VertexPaintedComponent>().vertexColors;
		for (auto& submesh : mesh->GetSubMeshes())
		{
			for (uint32_t index = submesh.vertexStartOffset; index < submesh.vertexStartOffset + submesh.vertexCount; index++)
			{
				auto& vertex = mesh->GetVertices().at(index);
				auto vPos = glm::vec3(paintedEnt.GetTransform() * submesh.transform * glm::vec4(vertex.position, 1));
				if (glm::distance2(vPos, m_brushPosition) < m_settings.billboardRange * m_settings.billboardRange)
				{
					auto& currentVertexColor = vertecies[index];

					glm::vec4 unpackedColor = Volt::Utility::UnpackUIntToUNormFloat4(currentVertexColor);

					if (m_settings.paintRedChannel)		ApplyColor(unpackedColor.x, color);
					if (m_settings.paintGreenChannel)	ApplyColor(unpackedColor.y, color);
					if (m_settings.paintBlueChannel)	ApplyColor(unpackedColor.z, color);
					if (m_settings.paintAlphaChannel)	ApplyColor(unpackedColor.w, color);

					currentVertexColor = Volt::Utility::PackUNormFloat4AsUInt(unpackedColor);
				}
			}
		}
	}
}

void VertexPainterPanel::ApplyColor(float& vertexColor, float paintColor)
{
	if (m_settings.ignoreIntencity)
	{
		vertexColor = paintColor;
		return;
	}

	if (vertexColor < paintColor * 0.99f)
	{
		vertexColor += paintColor * m_settings.intensity * m_deltaTime;
		if (vertexColor > 1)
			vertexColor = 1;
	}
	else if (vertexColor > paintColor * 0.99f)
	{
		vertexColor -= (1 - paintColor) * m_settings.intensity * m_deltaTime;
		if (vertexColor < 0)
			vertexColor = 0;
	}
}

void VertexPainterPanel::SetView(bool in_viewVertexColors)
{
	m_settings.viewEnabled = in_viewVertexColors;
	if (in_viewVertexColors)
	{
		for (auto entId : SelectionManager::GetSelectedEntities())
		{
			Volt::Entity entity = ex_scene->GetEntityFromUUID(entId);
			if (!entity.HasComponent<Volt::MeshComponent>()) continue;
			auto& meshComp = entity.GetComponent<Volt::MeshComponent>();

			std::pair<Volt::EntityID, Volt::AssetHandle> entry;
			entry.first = entId;
			entry.second = meshComp.material;
			m_originalMaterials.insert(entry);

			meshComp.material = Volt::AssetManager::GetAssetHandleFromFilePath("Editor/Materials/M_VisualizeVertexColors.vtmat");
		}
		return;
	}
	for (auto _pair : m_originalMaterials)
	{
		Volt::Entity entity = ex_scene->GetEntityFromUUID(_pair.first);
		if (!entity.HasComponent<Volt::MeshComponent>()) continue;
		auto& meshComp = entity.GetComponent<Volt::MeshComponent>();
		meshComp.material = _pair.second;
	}
	m_originalMaterials.clear();

}

