#pragma once

#include <Wire/Serialization.h>
#include <glm/glm.hpp>

namespace Volt
{
	SERIALIZE_COMPONENT((struct GTAOEffectComponent
	{
		PROPERTY(Name = Radius) float radius = 50.f;
		PROPERTY(Name = Radius Multiplier) float radiusMultiplier = 1.457f;
		PROPERTY(Name = Falloff Range) float falloffRange = 0.615f;
		PROPERTY(Name = Final Value Power) float finalValuePower = 2.2f;

		CREATE_COMPONENT_GUID("{18531357-75D2-4312-AE5C-9E0BE55FD070}"_guid);
	}), GTAOEffectComponent);

	SERIALIZE_COMPONENT((struct GlobalFogComponent
	{
		PROPERTY(Name = Anisotropy) float anisotropy = 0.f;
		PROPERTY(Name = Density) float density = 0.04f;
		PROPERTY(Name = Global Density) float globalDensity = 0.f;
		PROPERTY(Name = Global Color, SpecialType = Color) glm::vec3 globalColor = 1.f;

		CREATE_COMPONENT_GUID("{F3628A10-5E97-4CA4-AA08-2C6A3EB4FAD1}"_guid);
	}), GlobalFogComponent);
}
