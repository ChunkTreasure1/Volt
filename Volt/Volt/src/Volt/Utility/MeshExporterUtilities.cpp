#include "vtpch.h"
#include "MeshExporterUtilities.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Components/RenderingComponents.h"
#include "Volt/Components/CoreComponents.h"

namespace Volt
{
	Ref<Volt::Mesh> MeshExporterUtilities::CombineMeshes(Ref<Volt::Scene> scene, const std::vector<entt::entity>& entities, Ref<Volt::Material> material, float unitModifier)
	{
		std::vector<Volt::Vertex> v;
		std::vector<uint32_t> i;

		for (uint32_t entIndex = 0; entIndex < entities.size(); entIndex++)
		{
			auto entity = Volt::Entity(entities[entIndex], scene.get());

			auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(entity.GetComponent<Volt::MeshComponent>().GetHandle());

			auto vertexContainer = mesh->GetVertexContainer();
			auto indices = mesh->GetIndices();

			std::vector<Vertex> vertices;

			for (const auto& submesh : mesh->GetSubMeshes())
			{
				vertices.reserve(vertices.size() + submesh.vertexCount);

				for (uint32_t index = submesh.vertexStartOffset; index < submesh.vertexStartOffset + submesh.vertexCount; index++)
				{
					Vertex& vertex = vertices.emplace_back();
					vertex.position = entity.GetTransform() * submesh.transform * glm::vec4(vertexContainer.positions[index], 1.f);
					
					const auto& materialData = vertexContainer.materialData[index];

					const glm::vec2 encodedNormal = { materialData.normal.x / 255.f, materialData.normal.y / 255.f };
					const glm::vec3 normal = Utility::OctNormalDecode(encodedNormal);

					vertex.normal = normal;
					vertex.uv = materialData.texCoords;
				}
			}

			for (auto& ind : indices) 
			{ 
				ind += static_cast<uint32_t>(v.size()); 
			}

			v.insert(v.end(), vertices.begin(), vertices.end());
			i.insert(i.end(), indices.begin(), indices.end());
		}

		return (!v.empty()) ? CreateRef<Volt::Mesh>(v, i, material) : nullptr;
	}

	Ref<Volt::Mesh> MeshExporterUtilities::CombineMeshes(const std::vector<Ref<Volt::Mesh>>& meshes, const std::vector<glm::mat4>& transforms, Ref<Volt::Material> material)
	{
		std::vector<Volt::Vertex> v;
		std::vector<uint32_t> i;

		for (uint32_t meshIndex = 0; meshIndex < meshes.size(); meshIndex++)
		{
			auto mesh = meshes[meshIndex];

			auto vertexContainer = mesh->GetVertexContainer();
			auto indices = mesh->GetIndices();

			std::vector<Vertex> vertices;

			for (const auto& submesh : mesh->GetSubMeshes())
			{
				for (uint32_t index = submesh.vertexStartOffset; index < submesh.vertexStartOffset + submesh.vertexCount; index++)
				{
					Vertex& vertex = vertices.emplace_back();
					vertex.position = transforms[index] * submesh.transform * glm::vec4(vertexContainer.positions[index], 1.f);

					const auto& materialData = vertexContainer.materialData[index];

					const glm::vec2 encodedNormal = { materialData.normal.x / 255.f, materialData.normal.y / 255.f };
					const glm::vec3 normal = Utility::OctNormalDecode(encodedNormal);

					vertex.normal = normal;
					vertex.uv = materialData.texCoords;
				}
			}

			for (auto& ind : indices)
			{
				ind += static_cast<uint32_t>(v.size());
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
				auto handle = entities[entIndex].GetComponent<Volt::MeshComponent>().GetHandle();
				auto transform = entities[entIndex].GetComponent<Volt::TransformComponent>();
				auto asset = Volt::AssetManager::GetAsset<Volt::Mesh>(handle);

				if (asset)
				{
					Ref<Volt::Mesh> mesh = CreateRef<Volt::Mesh>(*asset);

					for (auto& vert : mesh->m_vertexContainer.positions)
					{
						auto v4 = glm::vec4(vert, 1.f);
						vert = transform.GetTransform() * v4;
					}

					meshes.emplace_back(mesh);
				}
			}
		}

		return meshes;
	}
}
