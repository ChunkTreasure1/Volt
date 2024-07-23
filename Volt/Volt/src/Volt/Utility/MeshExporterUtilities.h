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
		static Ref<Volt::Mesh> CombineMeshes(Ref<Volt::Scene> scene, const Vector<entt::entity>& entities, Ref<Volt::Material> material, float unitModifier = 1.f);
		static Ref<Volt::Mesh> CombineMeshes(const Vector<Ref<Volt::Mesh>>& meshes, const Vector<glm::mat4>& transforms, Ref<Volt::Material> material);
		static Vector<Ref<Volt::Mesh>> GetMeshes(const Vector<Volt::Entity>& entities);
	};
}
