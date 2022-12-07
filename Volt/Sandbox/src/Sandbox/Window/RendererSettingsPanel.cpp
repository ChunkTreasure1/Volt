#include "sbpch.h"
#include "RendererSettingsPanel.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/SceneRenderer.h>

RendererSettingsPanel::RendererSettingsPanel(Ref<Volt::SceneRenderer>& sceneRenderer)
	: EditorWindow("Renderer Settings"), mySceneRenderer(sceneRenderer)
{}

void RendererSettingsPanel::UpdateMainContent()
{
	if (ImGui::CollapsingHeader("Post Process"))
	{
		UI::PushId();
		ImGui::Separator();
		ImGui::TextUnformatted("HBAO");
		if (UI::BeginProperties("hbao"))
		{
			auto& hbaoSettings = mySceneRenderer->GetHBAOSettings();

			UI::Property("Enabled", hbaoSettings.enabled);
			UI::Property("Radius", hbaoSettings.radius);
			UI::Property("Intensity", hbaoSettings.intensity);
			UI::Property("Bias", hbaoSettings.bias);

			UI::EndProperties();
		}
		ImGui::Separator();

		ImGui::TextUnformatted("Bloom");
		if (UI::BeginProperties("bloom"))
		{
			auto& bloomSettings = mySceneRenderer->GetBloomSettings();

			UI::Property("Enabled", bloomSettings.enabled);
			UI::EndProperties();
		}
		ImGui::Separator();

		ImGui::TextUnformatted("FXAA");
		if (UI::BeginProperties("vignette"))
		{
			auto& fxaaSettings = mySceneRenderer->GetFXAASettings();

			UI::Property("Enabled", fxaaSettings.enabled);
			UI::EndProperties();
		}
		ImGui::Separator();

		ImGui::TextUnformatted("Vignette");
		if (UI::BeginProperties("vignette"))
		{
			auto& vignetteSettings = mySceneRenderer->GetVignetteSettings();

			bool changed = false;

			changed |= UI::Property("Enabled", vignetteSettings.enabled);
			changed |= UI::Property("Width", vignetteSettings.width);
			changed |= UI::Property("Sharpness", vignetteSettings.sharpness);
			changed |= UI::PropertyColor("Color Tint", vignetteSettings.color);
			
			if (changed)
			{
				mySceneRenderer->UpdateVignetteSettings();
			}
			UI::EndProperties();
		}
		ImGui::Separator();

		ImGui::TextUnformatted("Gamma");
		if (UI::BeginProperties("gamma"))
		{
			auto& gammaSettings = mySceneRenderer->GetGammaSettingss();

			UI::Property("Enabled", gammaSettings.enabled);
			UI::EndProperties();
		}

		UI::PopId();

	}
	UI::PushId();

	ImGui::TextUnformatted("Environment");
	if (UI::BeginProperties("env"))
	{
		UI::Property("Ambiance multiplier", Volt::Renderer::GetSettings().ambianceMultiplier);
		UI::EndProperties();
	}

	ImGui::Separator();
	ImGui::TextUnformatted("Camera");
	if (UI::BeginProperties("cam"))
	{
		UI::Property("Exposure", Volt::Renderer::GetSettings().exposure);
		UI::EndProperties();
	}

	UI::PopId();
}
