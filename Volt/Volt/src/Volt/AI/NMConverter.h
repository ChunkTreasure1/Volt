#pragma once
#include "Volt/Core/Base.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Components/Components.h"
#include "Volt/Scene/Entity.h"
#include <vector>

namespace Volt
{
	class NMConverter
	{
	public:
		static Ref<Mesh> CombineMeshes(const std::vector<Volt::Entity>& entities, float unitModifier = 1.f);
	};
}