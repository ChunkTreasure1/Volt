#pragma once

#include <Volt/Core/Base.h>
#include <Volt/Scene/Entity.h>

namespace Volt
{
	class SceneRendererNew;
	class Scene;
	class Camera;
}

namespace AssetBrowser
{
	class AssetItem;
}

class PreviewRenderer
{
public:
	PreviewRenderer();
	~PreviewRenderer();

	void RenderPreview(Weak<AssetBrowser::AssetItem> assetItem);

private:
	bool RenderMeshPreview(Weak<AssetBrowser::AssetItem> assetItem);
	bool RenderMaterialPreview(Weak<AssetBrowser::AssetItem> assetItem);

	Ref<Volt::SceneRendererNew> myPreviewRenderer;
	Ref<Volt::Scene> myPreviewScene;
	Ref<Volt::Camera> myCamera;

	Volt::Entity myEntity;
};
