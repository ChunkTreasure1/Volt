#pragma once
#include "Volt/Core/Base.h"

#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Scene/Entity.h"

#include <vector>

namespace Volt
{
	class Material;

	class MeshExporterUtilities
	{
	public:
		static Ref<Volt::Mesh> CombineMeshes(Ref<Volt::Scene> scene, const std::vector<entt::entity>& entities, Ref<Volt::Material> material, float unitModifier = 1.f);
		static Ref<Volt::Mesh> CombineMeshes(const std::vector<Ref<Volt::Mesh>>& meshes, const std::vector<glm::mat4>& transforms, Ref<Volt::Material> material);
		static std::vector<Ref<Volt::Mesh>> GetMeshes(const std::vector<Volt::Entity>& entities);
	};
}
