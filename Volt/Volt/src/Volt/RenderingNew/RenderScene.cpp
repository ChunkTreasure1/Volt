#include "vtpch.h"
#include "RenderScene.h"

namespace Volt
{
	RenderScene::RenderScene(Scene* sceneRef)
		: m_scene(sceneRef)
	{

	}
	
	void RenderScene::SetValid()
	{
		m_isInvalid = false;
	}

	void RenderScene::Invalidate()
	{
		m_isInvalid = true;
	}

	const UUID RenderScene::Register(Wire::EntityId entityId, Ref<Mesh> mesh, uint32_t subMeshIndex)
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
}
