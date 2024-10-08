#include "sbpch.h"
#include "Window/RendererSettingsPanel.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Renderer.h>

#include "Sandbox/Utility/EditorUtilities.h"

RendererSettingsPanel::RendererSettingsPanel(Ref<Volt::SceneRenderer>& sceneRenderer)
	: EditorWindow("Renderer Settings"), m_sceneRenderer(sceneRenderer)
{
}

void RendererSettingsPanel::UpdateMainContent()
{
	if (ImGui::Button("Invalidate Render Scene"))
	{
		m_sceneRenderer->Invalidate();
	}

	static const Vector<std::string> shadingStrings =
	{
		"Shaded",
		"Albedo",
		"Normals",
		"Metalness",
		"Roughness",
		"Emissive",
		"AO"
	};

	int32_t currentValue = static_cast<int32_t>(m_sceneRenderer->GetShadingMode());
	if (UI::Combo("Shading Mode", currentValue, shadingStrings))
	{
		m_sceneRenderer->SetShadingMode(static_cast<Volt::SceneRenderer::ShadingMode>(currentValue));
	}

	static const Vector<std::string> visualizationStrings =
	{
		"None",
		"VisualizeCascades",
		"VisualizeLightComplexity",
		"VisualizeMeshSDF"
	};

	currentValue = static_cast<int32_t>(m_sceneRenderer->GetVisualizationMode());
	if (UI::Combo("Visualization Mode", currentValue, visualizationStrings))
	{
		m_sceneRenderer->SetVisualizationMode(static_cast<Volt::SceneRenderer::VisualizationMode>(currentValue));
	}

	if (ImGui::Button("Enable"))
	{
		m_sceneRenderer->Enable();
	}

	//UI::Header("Settings");

	//bool changed = false;
	//auto& settings = mySceneRenderer->GetSettings();

	//UI::Header("Ambient Occlusion");

	//if (UI::BeginProperties("Rendering-AO"))
	//{
	//	const Vector<const char*> aoLevels = { "Low", "Medium", "High", "Ultra" };

	//	changed |= UI::Property("Enable AO", settings.enableAO);
	//	changed |= UI::ComboProperty("AO Quality", *(int32_t*)&settings.aoQuality, aoLevels);

	//	UI::EndProperties();
	//}

	//UI::Header("Shadows");

	//if (UI::BeginProperties("Rendering-Shadows"))
	//{
	//	const Vector<const char*> shadowLevels = { "Low", "Medium", "High" };

	//	changed |= UI::Property("Enable Shadows", settings.enableShadows);
	//	changed |= UI::ComboProperty("Shadow Resolution", *(int32_t*)&settings.shadowResolution, shadowLevels);

	//	UI::EndProperties();
	//}

	//UI::Header("Anti Aliasing");

	//if (UI::BeginProperties("Rendering-AA"))
	//{
	//	const Vector<const char*> aaTypes = { "FXAA", "TAA" };

	//	changed |= UI::Property("Enable AA", settings.enableAntiAliasing);
	//	changed |= UI::ComboProperty("AA Type", *(int32_t*)&settings.antiAliasing, aaTypes);

	//	UI::EndProperties();
	//}

	//UI::Header("Other");

	//if (UI::BeginProperties("Rendering-Other"))
	//{
	//	changed |= UI::Property("Render Scale", settings.renderScale, true, 0.1f, 4.f);
	//	changed |= UI::Property("Enable Bloom", settings.enableBloom);
	//	changed |= UI::Property("Enable UI", settings.enableUI);
	//	changed |= UI::Property("Enable Skybox", settings.enableSkybox);
	//	changed |= UI::Property("Enable Post Processing", settings.enablePostProcessing);
	//	
	//	UI::EndProperties();
	//}

	//if (changed)
	//{
	//	mySceneRenderer->ApplySettings();
	//}

	//if (ImGui::Button("Reload Ray Tracing"))
	//{
	//	mySceneRenderer->UpdateRayTracingScene();
	//}
}
