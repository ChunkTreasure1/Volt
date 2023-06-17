#include "vtpch.h"
#include "MeshExporterUtilities.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Components/Components.h"

namespace Volt
{
	Ref<Volt::Mesh> MeshExporterUtilities::CombineMeshes(Ref<Volt::Scene> scene, const std::vector<Wire::EntityId>& entities, Ref<Volt::Material> material, float unitModifier)
	{
		std::vector<Volt::Vertex> v;
		std::vector<uint32_t> i;

		for (uint32_t entIndex = 0; entIndex < entities.size(); entIndex++)
		{
			auto entity = Volt::Entity(entities[entIndex], scene.get());

			auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(entity.GetComponent<Volt::MeshComponent>().handle);

			auto vertices = mesh->GetVertices();
			auto indices = mesh->GetIndices();

			for (const auto& submesh : mesh->GetSubMeshes())
			{
				for (uint32_t index = submesh.vertexStartOffset; index < submesh.vertexStartOffset + submesh.vertexCount; index++)
				{
					vertices[index].position = entity.GetTransform() * submesh.transform * gem::vec4(vertices[index].position, 1.f);
				}
			}

			for (auto& ind : indices) 
			{ 
				ind += v.size(); 
			}

			v.insert(v.end(), vertices.begin(), vertices.end());
			i.insert(i.end(), indices.begin(), indices.end());
		}

		return (!v.empty()) ? CreateRef<Volt::Mesh>(v, i, material) : nullptr;
	}

	Ref<Volt::Mesh> MeshExporterUtilities::CombineMeshes(const std::vector<Ref<Volt::Mesh>>& meshes, const std::vector<gem::mat4>& transforms, Ref<Volt::Material> material)
	{
		std::vector<Volt::Vertex> v;
		std::vector<uint32_t> i;

		for (uint32_t meshIndex = 0; meshIndex < meshes.size(); meshIndex++)
		{
			auto mesh = meshes[meshIndex];

			auto vertices = mesh->GetVertices();
			auto indices = mesh->GetIndices();

			for (const auto& submesh : mesh->GetSubMeshes())
			{
				for (uint32_t index = submesh.vertexStartOffset; index < submesh.vertexStartOffset + submesh.vertexCount; index++)
				{
					vertices[index].position = transforms[meshIndex] * submesh.transform * gem::vec4(vertices[index].position, 1.f);
				}
			}

			for (auto& ind : indices)
			{
				ind += v.size();
			}

			v.insert(v.end(), vertices.begin(), vertices.end());
			i.insert(i.end(), indices.begin(), indices.end());
		}

		return (!v.empty()) ? CreateRef<Volt::Mesh>(v, i, material) : nullptr;
	}

	std::vector<Ref<Volt::Mesh>> MeshExporterUtilities::GetMeshes(const std::vector<Volt::Entity>& entities)
	{
		std::vector<Ref<Volt::Mesh>> meshes;

		for (uint32_t entIndex = 0; entIndex < entities.size(); entIndex++)
		{
			if (entities[entIndex].HasComponent<Volt::MeshComponent>())
			{
				auto handle = entities[entIndex].GetComponent<Volt::MeshComponent>().handle;
				auto transform = entities[entIndex].GetComponent<Volt::TransformComponent>();
				auto asset = Volt::AssetManager::GetAsset<Volt::Mesh>(handle);

				if (asset)
				{
					Ref<Volt::Mesh> mesh = CreateRef<Volt::Mesh>(*asset);

					for (auto& vert : mesh->myVertices)
					{
						auto v4 = gem::vec4(vert.position);
						v4.w = 1.f;
						vert.position = transform.GetTransform() * v4;
					}

					meshes.emplace_back(mesh);
				}
			}
		}

		return meshes;
	}
}
