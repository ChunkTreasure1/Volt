#include "sbpch.h"
#include "RendererSettingsPanel.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/RenderingNew/SceneRendererNew.h>

RendererSettingsPanel::RendererSettingsPanel(Ref<Volt::SceneRendererNew>& sceneRenderer)
	: EditorWindow("Renderer Settings"), mySceneRenderer(sceneRenderer)
{
}

void RendererSettingsPanel::UpdateMainContent()
{
	if (ImGui::Button("Invalidate Render Scene"))
	{
		mySceneRenderer->Invalidate();
	}

	//UI::Header("Settings");

	//bool changed = false;
	//auto& settings = mySceneRenderer->GetSettings();

	//UI::Header("Ambient Occlusion");

	//if (UI::BeginProperties("Rendering-AO"))
	//{
	//	const std::vector<const char*> aoLevels = { "Low", "Medium", "High", "Ultra" };

	//	changed |= UI::Property("Enable AO", settings.enableAO);
	//	changed |= UI::ComboProperty("AO Quality", *(int32_t*)&settings.aoQuality, aoLevels);

	//	UI::EndProperties();
	//}

	//UI::Header("Shadows");

	//if (UI::BeginProperties("Rendering-Shadows"))
	//{
	//	const std::vector<const char*> shadowLevels = { "Low", "Medium", "High" };

	//	changed |= UI::Property("Enable Shadows", settings.enableShadows);
	//	changed |= UI::ComboProperty("Shadow Resolution", *(int32_t*)&settings.shadowResolution, shadowLevels);

	//	UI::EndProperties();
	//}

	//UI::Header("Anti Aliasing");

	//if (UI::BeginProperties("Rendering-AA"))
	//{
	//	const std::vector<const char*> aaTypes = { "FXAA", "TAA" };

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
