#pragma once

#include <Wire/Serialization.h>
#include <GEM/gem.h>

namespace Volt
{
	SERIALIZE_COMPONENT((struct HeightFogComponent
	{
		PROPERTY(Name = Color, SpecialType = Color) gem::vec3 color = { 1.f, 1.f, 1.f };
		PROPERTY(Name = Strength) float strength = 1.f;
		PROPERTY(Name = Max Y) float maxY = 200.f;
		PROPERTY(Name = Min Y) float minY = 0.f;

		CREATE_COMPONENT_GUID("{85FB6122-FB03-412B-9C54-C241F9B12E15}"_guid);
	}), HeightFogComponent);

	SERIALIZE_COMPONENT((struct HBAOComponent
	{
		PROPERTY(Name = Radius) float radius = 1.f;
		PROPERTY(Name = Intensity) float intensity = 1.5f;
		PROPERTY(Name = Bias) float bias = 0.35f;

		CREATE_COMPONENT_GUID("{00CD576A-7C9B-4889-9B53-BA4AAFA1A0A8}"_guid);
	}), HBAOComponent);

	SERIALIZE_COMPONENT((struct BloomComponent
	{
		CREATE_COMPONENT_GUID("{5C29AC74-E4D4-43AD-99BD-C232FE9CDB2B}"_guid);
	}), BloomComponent);

	SERIALIZE_COMPONENT((struct FXAAComponent
	{
		CREATE_COMPONENT_GUID("{98033A5F-EE17-477C-A811-0E7CFA93D96C}"_guid);
	}), FXAAComponent);

	SERIALIZE_COMPONENT((struct VignetteComponent
	{
		PROPERTY(Name = Color) gem::vec3 color = { 0.f, 0.f, 0.f };
		PROPERTY(Name = Width) float width = 1.5f;
		PROPERTY(Name = Sharpness) float sharpness = 0.8f;

		CREATE_COMPONENT_GUID("{576E18AF-3817-4691-9B6A-392E65F6085A}"_guid);
	}), VignetteComponent);
}