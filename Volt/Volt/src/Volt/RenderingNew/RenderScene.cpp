#include "vtpch.h"
#include "RenderScene.h"

#include "Volt/Asset/Mesh/Mesh.h"

#include <VoltRHI/Buffers/StorageBuffer.h>

namespace Volt
{
	RenderScene::RenderScene(Scene* sceneRef)
		: m_scene(sceneRef)
	{

	}

	void RenderScene::PrepareForUpdate()
	{
		std::sort(std::execution::par, m_renderObjects.begin(), m_renderObjects.end(), [](const auto& lhs, const auto& rhs)
		{
			if (lhs.mesh < rhs.mesh)
			{
				return true;
			}

			if (lhs.mesh > rhs.mesh)
			{
				return false;
			}

			if (lhs.subMeshIndex < rhs.subMeshIndex)
			{
				return true;
			}

			if (lhs.subMeshIndex > rhs.subMeshIndex)
			{
				return false;
			}

			return false;
		});

		m_currentIndividualMeshCount = 0;
		m_individualMeshes.clear();
		m_vertexPositionViews.clear();
		m_vertexMaterialViews.clear();
		m_vertexAnimationViews.clear();
		m_indexBufferViews.clear();

		for (size_t i = 0; i < m_renderObjects.size(); i++)
		{
			if (i == 0)
			{
				m_currentIndividualMeshCount++;
				m_individualMeshes.push_back(m_renderObjects[i].mesh);

				AddMeshToViews(m_renderObjects[i].mesh);

			}
			else
			{
				if (m_renderObjects[i].mesh != m_renderObjects[i - 1].mesh)
				{
					m_currentIndividualMeshCount += static_cast<uint32_t>(m_renderObjects[i].mesh->GetSubMeshes().size());
					m_individualMeshes.push_back(m_renderObjects[i].mesh);

					AddMeshToViews(m_renderObjects[i].mesh);
				}
			}
		}
	}

	void RenderScene::SetValid()
	{
		m_isInvalid = false;
	}

	void RenderScene::Invalidate()
	{
		m_isInvalid = true;
	}

	const UUID RenderScene::Register(entt::entity entityId, Ref<Mesh> mesh, uint32_t subMeshIndex)
	{
		m_isInvalid = true;

		UUID newId = {};
		auto& newObj = m_renderObjects.emplace_back();

		newObj.id = newId;
		newObj.entity = entityId;
		newObj.mesh = mesh;
		newObj.subMeshIndex = subMeshIndex;

		return newId;
	}

	void RenderScene::Unregister(UUID id)
	{
		m_isInvalid = false;

		auto it = std::find_if(m_renderObjects.begin(), m_renderObjects.end(), [id](const auto& obj)
		{
			return obj.id == id;
		});

		if (it != m_renderObjects.end())
		{
			m_renderObjects.erase(it);
		}
	}

	void RenderScene::AddMeshToViews(Ref<Mesh> mesh)
	{
		m_vertexPositionViews.push_back(mesh->GetVertexPositionsBuffer()->GetView());
		m_vertexMaterialViews.push_back(mesh->GetVertexMaterialBuffer()->GetView());
		m_vertexAnimationViews.push_back(mesh->GetVertexAnimationBuffer()->GetView());
		m_indexBufferViews.push_back(mesh->GetIndexStorageBuffer()->GetView());
	}
}
