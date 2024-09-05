#include "sbpch.h"
#include "Window/GameUIEditorPanel.h"

#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/Theme.h"
#include "Sandbox/Utility/SelectionManager.h"

#include <Volt/GameUI/UIScene.h>
#include <Volt/GameUI/UIComponents.h>
#include <Volt/GameUI/UIWidget.h>
#include <InputModule/KeyCodes.h>

#include <Volt/Rendering/UISceneRenderer.h>

#include <Volt/Utility/UIUtility.h>

GameUIEditorPanel::GameUIEditorPanel()
	: EditorWindow("Game UI Editor", true)
{
	RegisterListener<Volt::WindowRenderEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnRenderEvent));
	RegisterListener<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnMouseButtonPressedEvent));
	RegisterListener<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnMouseButtonReleasedEvent));
	RegisterListener<Volt::MouseScrolledEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnMouseScrollEvent));
	RegisterListener<Volt::MouseMovedEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnMouseMovedEvent));

	m_uiScene = CreateRef<Volt::UIScene>();

	{
		auto widget = m_uiScene->CreateWidget();
		widget.AddComponent<Volt::UIImageComponent>();
	}

	Volt::UISceneRendererSpecification rendererSpec{};
	rendererSpec.scene = m_uiScene;
	rendererSpec.isEditor = true;

	m_uiSceneRenderer = CreateScope<Volt::UISceneRenderer>(rendererSpec);
}

void GameUIEditorPanel::UpdateContent()
{
	UpdateViewport();
	HandleSelection();

	UpdateHierarchy();
	UpdateDetails();
}

void GameUIEditorPanel::OnOpen()
{
	m_uiScene = CreateRef<Volt::UIScene>();

	{
		auto widget = m_uiScene->CreateWidget();
		widget.AddComponent<Volt::UIImageComponent>();
	}

	Volt::UISceneRendererSpecification rendererSpec{};
	rendererSpec.scene = m_uiScene;
	m_uiSceneRenderer = CreateScope<Volt::UISceneRenderer>(rendererSpec);
}

void GameUIEditorPanel::OnClose()
{
	m_uiScene = nullptr;
	m_uiSceneRenderer = nullptr;
	m_viewportImage = nullptr;
}

bool GameUIEditorPanel::OnRenderEvent(Volt::WindowRenderEvent& e)
{
	if (!m_viewportImage || static_cast<float>(m_viewportImage->GetWidth()) != m_viewportSize.x || static_cast<float>(m_viewportImage->GetHeight()) != m_viewportSize.y)
	{
		if (m_viewportSize.x > 0.f && m_viewportSize.y > 0.f)
		{
			CreateViewportImage(static_cast<uint32_t>(m_viewportSize.x), static_cast<uint32_t>(m_viewportSize.y));
		}
	}

	if (m_viewportImage)
	{
		const glm::vec2 halfSize = glm::vec2{ m_viewportImage->GetWidth(), m_viewportImage->GetHeight() } / 2.f * m_currentZoom;
		const auto projection = glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, 0.1f, 100.f) * glm::translate(glm::mat4{ 1.f }, glm::vec3{ m_currentPosition.x, m_currentPosition.y, 0.f });

		m_uiSceneRenderer->OnRender(m_viewportImage, projection);
	}

	return false;
}

bool GameUIEditorPanel::OnMouseScrollEvent(Volt::MouseScrolledEvent& e)
{
	if (m_viewportHovered)
	{
		m_currentZoom -= e.GetYOffset() * 0.1f;
		m_currentZoom = std::max(0.1f, m_currentZoom);
	}

	return false;
}

bool GameUIEditorPanel::OnMouseButtonPressedEvent(Volt::MouseButtonPressedEvent& e)
{
	if (m_viewportHovered && e.GetMouseButton() == VT_MOUSE_BUTTON_MIDDLE)
	{
		m_middleMouseDown = true;
	}

	return false;
}

bool GameUIEditorPanel::OnMouseButtonReleasedEvent(Volt::MouseButtonReleasedEvent& e)
{
	if (e.GetMouseButton() == VT_MOUSE_BUTTON_MIDDLE)
	{
		m_middleMouseDown = false;
		m_previousMousePosition = 0.f;
	}

	return false;
}

bool GameUIEditorPanel::OnMouseMovedEvent(Volt::MouseMovedEvent& e)
{
	m_currentMousePosition = glm::vec2(e.GetX(), e.GetY());

	if (!m_middleMouseDown)
	{
		return false;
	}

	if (glm::all(glm::epsilonEqual(m_previousMousePosition, glm::vec2{ 0.f }, glm::epsilon<float>())))
	{
		m_previousMousePosition = { e.GetX(), e.GetY() };
	}

	m_currentPosition += (m_previousMousePosition - m_currentMousePosition) * glm::vec2{ -1.f, 1.f } * m_currentZoom;
	m_previousMousePosition = { e.GetX(), e.GetY() };

	return false;
}

glm::vec2 GameUIEditorPanel::GetViewportLocalPosition(const ImVec2& mousePos)
{
	auto [mx, my] = mousePos;
	mx -= m_viewportBounds[0].x;
	my -= m_viewportBounds[0].y;

	glm::vec2 perspectiveSize = m_viewportBounds[1] - m_viewportBounds[0];
	glm::vec2 result = { mx, my };

	return result;
}

void GameUIEditorPanel::UpdateViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

	ImGui::Begin("Viewport##gameUIEditor");

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	m_viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	m_viewportMouseCoords = GetViewportLocalPosition(ImGui::GetMousePos());

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	if (m_viewportSize != (*(glm::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		m_viewportSize = { viewportSize.x, viewportSize.y };
	}

	if (m_viewportImage)
	{
		ImGui::Image(UI::GetTextureID(m_viewportImage), viewportSize);
	}

	m_viewportHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

	ImGui::End();
	ImGui::PopStyleVar(3);
}

void GameUIEditorPanel::UpdateHierarchy()
{
	//ImGui::SetNextWindowClass(GetWindowClass());
	if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());
	
		UI::PushID();
		ImGui::BeginChild("Main", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			static std::string searchQuery;
			bool hasQuery;

			EditorUtils::SearchBar(searchQuery, hasQuery, false);

			UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
			ImGui::BeginChild("Scrollable", ImGui::GetContentRegionAvail());
			{

			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
		UI::PopID();

	}
	ImGui::End();
}

void GameUIEditorPanel::UpdateDetails()
{
	const auto& selectedWidgets = SelectionManager::GetSelectedEntities(SelectionContext::GameUIEditor);

	//ImGui::SetNextWindowClass(GetWindowClass());
	if (ImGui::Begin("Details", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		// #TODO_Ivar: Handle multiple widgets
		if (selectedWidgets.empty() || selectedWidgets.size() > 1)
		{
			ImGui::End();
			return;
		}

		auto selectedWidget = m_uiScene->GetWidgetFromUUID(selectedWidgets.front());

		UI::PushID();
		UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
		ImGui::BeginChild("Main", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			if (selectedWidget.HasComponent<Volt::UITagComponent>())
			{
				auto& comp = selectedWidget.GetComponent<Volt::UITagComponent>();
				if (UI::BeginProperties("Tag"))
				{
					UI::Property("Tag", comp.tag);
					UI::EndProperties();
				}
			}

			if (selectedWidget.HasComponent<Volt::UITransformComponent>() && UI::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& comp = selectedWidget.GetComponent<Volt::UITransformComponent>();
				if (UI::BeginProperties("Transform"))
				{
					UI::PropertyAxisColor("Position", comp.position);
					UI::PropertyAxisColor("Size", comp.size, 100.f);
					UI::PropertyAxisColor("Alignment", comp.alignment);
					UI::Property("Rotation", comp.rotation);
					UI::Property("Z Order", comp.zOrder);
					UI::EndProperties();
				}
			}

			if (selectedWidget.HasComponent<Volt::UIImageComponent>() && UI::CollapsingHeader("Image", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& comp = selectedWidget.GetComponent<Volt::UIImageComponent>();
				if (UI::BeginProperties("Image"))
				{
					EditorUtils::Property("Image", comp.imageHandle, AssetTypes::Texture);
					UI::PropertyColor("Tint", comp.tint);
					UI::Property("Alpha", comp.alpha);

					UI::EndProperties();
				}
			}
		}
		ImGui::EndChild();
		UI::PopID();
	}
	ImGui::End();
}

void GameUIEditorPanel::HandleSelection()
{
	if (!m_viewportHovered)
	{
		return;
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
		
		int32_t mouseX = (int32_t)m_viewportMouseCoords.x;
		int32_t mouseY = (int32_t)m_viewportMouseCoords.y;
	
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int32_t)viewportSize.x && mouseY < (int32_t)viewportSize.y)
		{
			uint32_t pixelData = m_uiSceneRenderer->GetIDImage()->ReadPixel<uint32_t>(static_cast<uint32_t>(mouseX), static_cast<uint32_t>(mouseY), 0u);
			const bool multiSelect = Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT);
			const bool deselect = Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL);

			if (!multiSelect && !deselect)
			{
				SelectionManager::DeselectAll(SelectionContext::GameUIEditor);
			}

			Volt::UIWidget widget = m_uiScene->GetWidgetFromUUID(pixelData);

			if (widget.IsValid())
			{
				if (deselect)
				{
					SelectionManager::Deselect(widget.GetID(), SelectionContext::GameUIEditor);
				}
				else
				{
					SelectionManager::Select(widget.GetID(), SelectionContext::GameUIEditor);
				}
			}
		}
	}
}

void GameUIEditorPanel::CreateViewportImage(const uint32_t width, const uint32_t height)
{
	Volt::RHI::ImageSpecification spec{};
	spec.width = width;
	spec.height = height;
	spec.usage = Volt::RHI::ImageUsage::AttachmentStorage;
	spec.generateMips = false;
	spec.format = Volt::RHI::PixelFormat::R8G8B8A8_UNORM;
	spec.debugName = "Viewport Image";

	m_viewportImage = Volt::RHI::Image::Create(spec);
}
