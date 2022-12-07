#include "vtpch.h"
#include "NMConverter.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Scene/Entity.h"

namespace Volt
{
	Ref<Volt::Mesh> NMConverter::CombineMeshes(const std::vector<Volt::Entity>& entities, float unitModifier)
	{
		std::vector<Vertex> v;
		std::vector<uint32_t> i;
		Ref<Material> m = AssetManager::GetAsset<Volt::Material>("Assets/Materials/M_BloomTest.vtmat"); // #SAMUEL_TODO: Change to appropriate material

		for (uint32_t entIndex = 0; entIndex < entities.size(); entIndex++)
		{
			auto mesh = AssetManager::GetAsset<Mesh>(entities[entIndex].GetComponent<MeshComponent>().handle);
			if (!mesh || !mesh->IsValid())
			{
				continue;
			}

			auto transform = entities[entIndex].GetComponent<TransformComponent>();

			auto vertices = mesh->GetVertices();
			auto indices = mesh->GetIndices();

			for (auto& vert : vertices) 
			{
				auto v4 = gem::vec4(vert.position);
				v4.w = 1.f;
				vert.position = transform.GetTransform() * v4 * unitModifier;
			}
			for (auto& ind : indices) { ind += v.size(); }
			
			v.insert(v.end(), vertices.begin(), vertices.end());
			i.insert(i.end(), indices.begin(), indices.end());
		}

		return (!v.empty()) ? CreateRef<Mesh>(v, i, m) : nullptr;
	}
}