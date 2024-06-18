#include "sbpch.h"
#include "GameUIEditorPanel.h"

#include <Volt/GameUI/UIScene.h>
#include <Volt/GameUI/UIComponents.h>
#include <Volt/GameUI/UIWidget.h>

#include <Volt/Rendering/UISceneRenderer.h>

#include <Volt/Utility/UIUtility.h>

GameUIEditorPanel::GameUIEditorPanel()
	: EditorWindow("Game UI Editor", true)
{
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
}

void GameUIEditorPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnRenderEvent));
	dispatcher.Dispatch<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnMouseButtonPressedEvent));
	dispatcher.Dispatch<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnMouseButtonReleasedEvent));
	dispatcher.Dispatch<Volt::MouseScrolledEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnMouseScrollEvent));
	dispatcher.Dispatch<Volt::MouseMovedEvent>(VT_BIND_EVENT_FN(GameUIEditorPanel::OnMouseMovedEvent));
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

bool GameUIEditorPanel::OnRenderEvent(Volt::AppRenderEvent& e)
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
	if (!m_middleMouseDown)
	{
		return false;
	}

	if (glm::all(glm::epsilonEqual(m_previousMousePosition, glm::vec2{ 0.f }, glm::epsilon<float>())))
	{
		m_previousMousePosition = { e.GetX(), e.GetY() };
	}

	m_currentPosition += (m_previousMousePosition - glm::vec2(e.GetX(), e.GetY())) * glm::vec2{ -1.f, 1.f } * m_currentZoom;
	m_previousMousePosition = { e.GetX(), e.GetY() };

	return false;
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

void GameUIEditorPanel::CreateViewportImage(const uint32_t width, const uint32_t height)
{
	Volt::RHI::ImageSpecification spec{};
	spec.width = width;
	spec.height = height;
	spec.usage = Volt::RHI::ImageUsage::AttachmentStorage;
	spec.generateMips = false;
	spec.format = Volt::RHI::PixelFormat::R8G8B8A8_UNORM;
	spec.debugName = "Viewport Image";

	m_viewportImage = Volt::RHI::Image2D::Create(spec);
}
