#pragma once

#include "Volt/Asset/Rendering/MaterialTable.h"

#include "Volt/Rendering/RenderObject.h"

#include <EntitySystem/EntityHelper.h>

VT_DECLARE_LOG_CATEGORY(LogScenePrimitiveData, LogVerbosity::Trace);

namespace Volt
{
	class Mesh;
	class RenderScene;

	struct ScenePrimitiveDescription
	{
		AssetHandle primitiveMesh;
		Vector<AssetHandle> materials;
	};

	class ScenePrimitiveData
	{
	public:
		ScenePrimitiveData(const EntityID& entityId, RenderScene* renderScene);
		~ScenePrimitiveData();

		void InitializeFromDescription(const ScenePrimitiveDescription& description);
		void Invalidate();

	private:
		void CreateScenePrimitives();
		void DestroyScenePrimitives();

		Ref<Mesh> m_primitiveMesh;
		MaterialTable m_primitiveMaterialTable;

		EntityID m_relatedEntity;
		RenderScene* m_renderScene;

		Vector<RenderObjectID> m_renderObjects;

		UUID64 m_meshChangedCallbackID = 0;
	};
}
