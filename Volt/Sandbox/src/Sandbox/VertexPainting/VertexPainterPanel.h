#pragma once
#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Window/EditorWindow.h"
#include <Volt/Events/KeyEvent.h>

class VertexPainterPanel : public EditorWindow
{
public:
	VertexPainterPanel(Ref<Volt::Scene>& in_scene, Ref<EditorCameraController>& in_cc);
	~VertexPainterPanel();

	void UpdateMainContent() override;
	bool OnViewportResizeEvent(Volt::ViewportResizeEvent& e);
	virtual void OnEvent(Volt::Event& e) override;
	void SetBrushPosition(const gem::vec3& in_position) { m_brushPosition = in_position; }
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
		gem::vec3 dir{ 1,0,0 };
		gem::vec3 pos{ 0,0,0 };
	}ray;

	struct MaterialSettings
	{
		Volt::AssetHandle singleMat = 0;
		std::unordered_map<Volt::AssetHandle, Volt::AssetHandle> matConversionRegistry;
	}m_materialSettings;

private:
	float m_deltaTime = 0;

	gem::vec2ui myViewportPosition;
	gem::vec2ui myViewportSize;
	gem::vec3 m_brushPosition = { 0,0,0 };
	Ref<Volt::Scene>& ex_scene;
	Ref<EditorCameraController>& ex_cameraController;

	std::unordered_map<Wire::EntityId, Volt::AssetHandle> m_originalMaterials;
};
