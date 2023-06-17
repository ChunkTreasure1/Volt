#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Scene/Scene.h"

#include <Wire/Serialization.h>
#include <gem/gem.h>

namespace Volt
{
	SERIALIZE_COMPONENT((struct PointLightComponent
	{
		PROPERTY(Name = Intensity) float intensity = 1.f;
		PROPERTY(Name = Radius) float radius = 100.f;
		PROPERTY(Name = Falloff) float falloff = 1.f;
		PROPERTY(Name = Color, SpecialType = Color) gem::vec3 color = { 1.f, 1.f, 1.f };
		PROPERTY(Name = Cast Shadows) bool castShadows = false;
		PROPERTY(Name = Ambient Only) bool ambientOnly = false;

		CREATE_COMPONENT_GUID("{A30A8848-A30B-41DD-80F9-4E163C01ABC2}"_guid)
	}), PointLightComponent);

	SERIALIZE_COMPONENT((struct SpotLightComponent
	{
		PROPERTY(Name = Intensity) float intensity = 1.f;
		PROPERTY(Name = Angle) float angle = 45.f;
		PROPERTY(Name = Range) float range = 100.f;
		PROPERTY(Name = Angle Attenuation) float angleAttenuation = 1.f;
		PROPERTY(Name = Falloff) float falloff = 1.f;
		PROPERTY(Name = Color, SpecialType = Color) gem::vec3 color = { 1.f, 1.f, 1.f };
		PROPERTY(Name = Cast Shadows) bool castShadows = false;

		CREATE_COMPONENT_GUID("{D35F915F-53E5-4E15-AE5B-769F4D79B6F8}"_guid)
	}), SpotLightComponent);

	SERIALIZE_COMPONENT((struct SphereLightComponent
	{
		PROPERTY(Name = Intensity) float intensity = 1.f;
		PROPERTY(Name = Radius) float radius = 50.f;
		PROPERTY(Name = Color, SpecialType = Color) gem::vec3 color = { 1.f, 1.f, 1.f };

		CREATE_COMPONENT_GUID("{0D0CEEE2-A331-442A-BB4B-FBDB8E06C692}"_guid)
	}), SphereLightComponent);

	SERIALIZE_COMPONENT((struct RectangleLightComponent
	{
		PROPERTY(Name = Intensity) float intensity = 1.f;
		PROPERTY(Name = Color, SpecialType = Color) gem::vec3 color = { 1.f, 1.f, 1.f };
		PROPERTY(Name = Width) float width = 50.f;
		PROPERTY(Name = Height) float height = 50.f;

		CREATE_COMPONENT_GUID("{5AEF9201-4A86-45F1-85F3-E95577E45BF2}"_guid)
	}), RectangleLightComponent);

	SERIALIZE_COMPONENT((struct DirectionalLightComponent
	{
		PROPERTY(Name = Intensity) float intensity = 1.f;
		PROPERTY(Name = Color, SpecialType = Color) gem::vec3 color = { 1.f, 1.f, 1.f };

		PROPERTY(Name = Light Size) float lightSize = 1.f;

		PROPERTY(Name = Soft Shadows) bool softShadows = true;
		PROPERTY(Name = Cast Shadows) bool castShadows = true;

		CREATE_COMPONENT_GUID("{EC5514FF-9DE7-44CA-BCD9-8A9F08883F59}"_guid)
	}), DirectionalLightComponent);

	SERIALIZE_COMPONENT((struct SkylightComponent
	{
		PROPERTY(Name = Environment Map, SpecialType = Asset, AssetType = Texture) AssetHandle environmentHandle = Asset::Null();
		PROPERTY(Name = Intensity) float intensity = 1.f;
		PROPERTY(Name = LOD) float lod = 0.f;

		PROPERTY(Name = Turbidity) float turbidity = 2.f;
		PROPERTY(Name = Azimuth) float azimuth = 0.f;
		PROPERTY(Name = Inclination) float inclination = 0.f;

		PROPERTY(Name = Show) bool show = true;

		SceneEnvironment currentSceneEnvironment;
		AssetHandle lastEnvironmentHandle = Asset::Null();

		CREATE_COMPONENT_GUID("{29F75381-2873-4734-A074-3F3640E54C84}"_guid);
	}), SkylightComponent);
}
