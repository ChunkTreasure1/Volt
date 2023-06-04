#pragma once

namespace Volt
{
	enum class ShadowResolutionSetting : int32_t
	{
		Low = 0,
		Medium = 1,
		High = 2
	};

	enum class AntiAliasingSetting
	{
		FXAA = 0,
		TAA = 1
	};

	enum class AOQualitySetting : int32_t
	{
		Low = 0,
		Medium,
		High,
		Ultra
	};

	struct SceneRendererSettings
	{
		ShadowResolutionSetting shadowResolution = ShadowResolutionSetting::Medium;
		AntiAliasingSetting antiAliasing = AntiAliasingSetting::FXAA;
		AOQualitySetting aoQuality = AOQualitySetting::Ultra;

		float renderScale = 1.f;

		///// Render Techniques /////
		bool enableShadows = true;
		bool enableAO = true;
		bool enableBloom = true;
		bool enableAntiAliasing = false;
		bool enableUI = false;
		bool enableSkybox = true;
		bool enablePostProcessing = false;
		bool enableRayTracing = false;
		bool enableVolumetricFog = false;

		///// Debug/Editor /////
		bool enableDebugRenderer = false;
		bool enableOutline = false;
		bool enableIDRendering = false;
		bool enableGrid = false;
			
		inline static const gem::vec2ui GetResolutionFromShadowSetting(ShadowResolutionSetting setting)
		{
			switch (setting)
			{
				case ShadowResolutionSetting::Low: return { 1024 };
				case ShadowResolutionSetting::Medium: return { 2048 };
				case ShadowResolutionSetting::High: return { 4096 };
			}

			return { 2048 };
		}
	};
}
