#pragma once
#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Window/EditorWindow.h"

#include <InputModule/Events/KeyboardEvents.h>
#include <WindowModule/Events/WindowEvents.h>
#include <Volt/Public/Events/ApplicationEvents.h>


#include <Volt/Scene/EntityID.h>

class VertexPainterPanel : public EditorWindow
{
public:
	VertexPainterPanel(Ref<Volt::Scene>& in_scene, Ref<EditorCameraController>& in_cc);
	~VertexPainterPanel();

	void UpdateMainContent() override;
	bool OnViewportResizeEvent(Volt::ViewportResizeEvent& e);
	void SetBrushPosition(const glm::vec3& in_position) { m_brushPosition = in_position; }
	void OnClose() override { SetView(false); }

	bool BrushUpdate();
	void PanelDraw();
	void BillboardDraw();
	void CameraDraw();

	bool UpdateDeltaTime(Volt::AppUpdateEvent deltaTime) { m_deltaTime = deltaTime.GetTimestep(); return false; }

	bool AddPainted(Volt::Entity entity);
	void Paint(float color);
	void ApplyColor(float& vertexColor, float paintColor);

	void SetView(bool in_viewVertexColors);

	struct Settings
	{
		bool paintRedChannel = true;
		bool paintGreenChannel = true;
		bool paintBlueChannel = true;
		bool paintAlphaChannel = true;
		bool isSelecting = true;
		bool viewEnabled = false;
		bool ignoreIntencity = false;

		enum class eView
		{
			RED,
			GREEN,
			BLUE,
			ALPHA,
			ALL
		} view = eView::ALL;

		float brushRadius = 50;
		float billboardRange = 100;
		float billboadScalar = 1;
		float intensity = 1.0f;
		float billboardAlpha = 0.5f;

		float paintColor = 1;
		float eraseColor = 0;
	} m_settings;

	struct Ray
	{
		glm::vec3 dir{ 1,0,0 };
		glm::vec3 pos{ 0,0,0 };
	}ray;

	struct MaterialSettings
	{
		Volt::AssetHandle singleMat = 0;
		std::unordered_map<Volt::AssetHandle, Volt::AssetHandle> matConversionRegistry;
	}m_materialSettings;

private:
	float m_deltaTime = 0;

	glm::uvec2 myViewportPosition;
	glm::uvec2 myViewportSize;
	glm::vec3 m_brushPosition = { 0,0,0 };
	Ref<Volt::Scene>& ex_scene;
	Ref<EditorCameraController>& ex_cameraController;

	std::unordered_map<Volt::EntityID, Volt::AssetHandle> m_originalMaterials;
};

