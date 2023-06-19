#pragma once

#include <string>
#include <entt.hpp>

#include "ComponentRegistry.h"

namespace Volt
{
	struct TestComponent
	{
		AssetHandle meshHandle = Asset::Null();
		glm::vec3 color = { 1.f };

		ENTT_COMPONENT(TestComponent);
		ENTT_PROPERTY(TestComponent, meshHandle, "Handle", SpecialType = eST_Asset, AssetType = Mesh);
		ENTT_PROPERTY(TestComponent, color, "Color", SpecialType = eST_Color);
	};
}
