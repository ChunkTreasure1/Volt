#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Scene/Scene.h"

#include <Wire/Serialization.h>
#include <GEM/gem.h>

namespace Volt
{
	SERIALIZE_COMPONENT((struct PointLightComponent
	{
		PROPERTY(Name = Intensity) float intensity = 1.f;
		PROPERTY(Name = Radius) float radius = 100.f;
		PROPERTY(Name = Falloff) float falloff = 1.f;
		PROPERTY(Name = Far plane) float farPlane = 100.f;
		PROPERTY(Name = Color, SpecialType = Color) gem::vec3 color = { 1.f, 1.f, 1.f };
		PROPERTY(Name = Cast Shadows) bool castShadows = false;

		CREATE_COMPONENT_GUID("{A30A8848-A30B-41DD-80F9-4E163C01ABC2}"_guid)
	}), PointLightComponent);

	SERIALIZE_COMPONENT((struct DirectionalLightComponent
	{
		PROPERTY(Name = Intensity) float intensity = 1.f;
		PROPERTY(Name = Color, SpecialType = Color) gem::vec3 color = { 1.f, 1.f, 1.f };
		PROPERTY(Name = Cast Shadows) bool castShadows = true;

		CREATE_COMPONENT_GUID("{EC5514FF-9DE7-44CA-BCD9-8A9F08883F59}"_guid)
	}), DirectionalLightComponent);

	SERIALIZE_COMPONENT((struct SkylightComponent
	{
		PROPERTY(Name = Environment Map, SpecialType = Asset, AssetType = Texture) AssetHandle environmentHandle = Asset::Null();
		PROPERTY(Name = Intensity) float intensity = 1.f;
		PROPERTY(Name = LOD) float lod = 0.f;
		PROPERTY(Name = Show) bool show = true;

		AssetHandle lastEnvironmentHandle = Asset::Null();
		SceneEnvironment currentSceneEnvironment;

		CREATE_COMPONENT_GUID("{29F75381-2873-4734-A074-3F3640E54C84}"_guid);
	}), SkylightComponent);
}