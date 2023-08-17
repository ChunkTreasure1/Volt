#include "vtpch.h"
#include "Components.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/RenderingNew/RenderScene.h"

namespace Volt
{
	void MeshComponent::SetMesh(AssetHandle handle)
	{
		auto renderScene = m_entity.GetScene()->GetRenderScene();

		for (const auto& uuid : renderObjectIds)
		{
			renderScene->Unregister(uuid);
		}

		renderObjectIds.clear();

		Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle);
		if (!mesh || !mesh->IsValid())
		{
			return;
		}

		for (size_t i = 0; i < mesh->GetSubMeshes().size(); i++)
		{
			auto uuid = renderScene->Register(m_entity.GetId(), mesh, static_cast<uint32_t>(i));
			renderObjectIds.emplace_back(uuid);
		}
	}
}
