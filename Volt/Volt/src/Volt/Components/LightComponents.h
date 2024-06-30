#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Reflection/ComponentReflection.h"
#include "Volt/Scene/Reflection/ComponentRegistry.h"

#include <glm/glm.hpp>

namespace Volt
{
	struct PointLightComponent
	{
		float intensity = 1.f;
		float radius = 100.f;
		float falloff = 1.f;
		glm::vec3 color = { 1.f, 1.f, 1.f };
		bool castShadows = false;
		bool ambientOnly = false;

		static void ReflectType(TypeDesc<PointLightComponent>& reflect)
		{
			reflect.SetGUID("{A30A8848-A30B-41DD-80F9-4E163C01ABC2}"_guid);
			reflect.SetLabel("Point Light Component");
			reflect.AddMember(&PointLightComponent::intensity, "intensity", "Intensity", "", 1.f);
			reflect.AddMember(&PointLightComponent::radius, "radius", "Radius", "", 100.f);
			reflect.AddMember(&PointLightComponent::falloff, "falloff", "Falloff", "", 1.f);
			reflect.AddMember(&PointLightComponent::color, "color", "Color", "", glm::vec3{ 1.f }, ComponentMemberFlag::Color3);
			reflect.AddMember(&PointLightComponent::castShadows, "castShadows", "Cast Shadows", "", false);
			reflect.AddMember(&PointLightComponent::ambientOnly, "Ambient Only", "Ambient Only", "", false);
		}

		REGISTER_COMPONENT(PointLightComponent);
	};

	struct SpotLightComponent
	{
		float intensity = 1.f;
		float angle = 45.f;
		float range = 100.f;
		float angleAttenuation = 1.f;
		float falloff = 1.f;
		glm::vec3 color = { 1.f, 1.f, 1.f };
		bool castShadows = false;

		static void ReflectType(TypeDesc<SpotLightComponent>& reflect)
		{
			reflect.SetGUID("{D35F915F-53E5-4E15-AE5B-769F4D79B6F8}"_guid);
			reflect.SetLabel("Spot Light Component");
			reflect.AddMember(&SpotLightComponent::intensity, "intensity", "Intensity", "", 1.f);
			reflect.AddMember(&SpotLightComponent::angle, "angle", "Angle", "", 45.f);
			reflect.AddMember(&SpotLightComponent::range, "range", "Range", "", 100.f);
			reflect.AddMember(&SpotLightComponent::angleAttenuation, "angleAttenuation", "Angle Attenuation", "", 1.f);
			reflect.AddMember(&SpotLightComponent::falloff, "falloff", "Falloff", "", 1.f);
			reflect.AddMember(&SpotLightComponent::color, "color", "Color", "", glm::vec3{ 1.f }, ComponentMemberFlag::Color3);
			reflect.AddMember(&SpotLightComponent::castShadows, "castShadows", "Cast Shadows", "", false);
		}

		REGISTER_COMPONENT(SpotLightComponent);
	};

	struct SphereLightComponent
	{
		float intensity = 1.f;
		float radius = 50.f;
		glm::vec3 color = { 1.f, 1.f, 1.f };

		static void ReflectType(TypeDesc<SphereLightComponent>& reflect)
		{
			reflect.SetGUID("{0D0CEEE2-A331-442A-BB4B-FBDB8E06C692}"_guid);
			reflect.SetLabel("Sphere Light Component");
			reflect.AddMember(&SphereLightComponent::intensity, "intensity", "Intensity", "", 1.f);
			reflect.AddMember(&SphereLightComponent::radius, "radius", "Radius", "", 50.f);
			reflect.AddMember(&SphereLightComponent::color, "color", "Color", "", glm::vec3{ 1.f }, ComponentMemberFlag::Color3);
		}

		REGISTER_COMPONENT(SphereLightComponent);
	};

	struct RectangleLightComponent
	{
		float intensity = 1.f;
		glm::vec3 color = { 1.f, 1.f, 1.f };
		float width = 50.f;
		float height = 50.f;

		static void ReflectType(TypeDesc<RectangleLightComponent>& reflect)
		{
			reflect.SetGUID("{5AEF9201-4A86-45F1-85F3-E95577E45BF2}"_guid);
			reflect.SetLabel("Rectangle Light Component");
			reflect.AddMember(&RectangleLightComponent::intensity, "intensity", "Intensity", "", 1.f);
			reflect.AddMember(&RectangleLightComponent::color, "color", "Color", "", glm::vec3{ 1.f }, ComponentMemberFlag::Color3);
			reflect.AddMember(&RectangleLightComponent::width, "width", "Width", "", 50.f);
			reflect.AddMember(&RectangleLightComponent::height, "height", "Height", "", 50.f);
		}

		REGISTER_COMPONENT(RectangleLightComponent);
	};

	struct DirectionalLightComponent
	{
		float intensity = 1.f;
		glm::vec3 color = { 1.f, 1.f, 1.f };
		float lightSize = 1.f;
		bool softShadows = true;
		bool castShadows = true;

		static void ReflectType(TypeDesc<DirectionalLightComponent>& reflect)
		{
			reflect.SetGUID("{EC5514FF-9DE7-44CA-BCD9-8A9F08883F59}"_guid);
			reflect.SetLabel("Directional Light Component");
			reflect.AddMember(&DirectionalLightComponent::intensity, "intensity", "Intensity", "", 1.f);
			reflect.AddMember(&DirectionalLightComponent::color, "color", "Color", "", glm::vec3{ 1.f }, ComponentMemberFlag::Color3);
			reflect.AddMember(&DirectionalLightComponent::lightSize, "lightSize", "Light Size", "", 1.f);
			reflect.AddMember(&DirectionalLightComponent::softShadows, "softShadows", "Soft Shadows", "", true);
			reflect.AddMember(&DirectionalLightComponent::castShadows, "castShadows", "Cast Shadows", "", true);
		}

		REGISTER_COMPONENT(DirectionalLightComponent);
	};

	struct SkylightComponent
	{
		AssetHandle environmentHandle = Asset::Null();
		float intensity = 1.f;
		float lod = 0.f;
		float turbidity = 2.f;
		float azimuth = 0.f;
		float inclination = 0.f;
		bool show = true;

		SceneEnvironment currentSceneEnvironment;
		AssetHandle lastEnvironmentHandle = Asset::Null();

		static void ReflectType(TypeDesc<SkylightComponent>& reflect)
		{
			reflect.SetGUID("{29F75381-2873-4734-A074-3F3640E54C84}"_guid);
			reflect.SetLabel("Skylight Component");
			reflect.AddMember(&SkylightComponent::environmentHandle, "environmentHandle", "Environment", "", Asset::Null(), AssetType::Texture);
			reflect.AddMember(&SkylightComponent::intensity, "intensity", "Intensity", "", 1.f);
			reflect.AddMember(&SkylightComponent::lod, "lod", "LOD", "", 0.f);
			reflect.AddMember(&SkylightComponent::turbidity, "turbidity", "Turbidity", "", 2.f);
			reflect.AddMember(&SkylightComponent::azimuth, "azimuth", "Azimuth", "", 0.f);
			reflect.AddMember(&SkylightComponent::inclination, "inclination", "Inclination", "", 0.f);
			reflect.AddMember(&SkylightComponent::show, "show", "Show", "", true);
		}

		REGISTER_COMPONENT(SkylightComponent);
	};
}
