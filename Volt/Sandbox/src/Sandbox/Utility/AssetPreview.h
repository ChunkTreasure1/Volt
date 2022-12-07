#pragma once

#include <Volt/Asset/Asset.h>
#include <Volt/Scene/Entity.h>

namespace Volt
{
	class Scene;
	class SceneRenderer;
	class Image2D;
	class Camera;
}

class AssetPreview
{
public:
	AssetPreview(const std::filesystem::path& path);

	void Render();
	const Ref<Volt::Image2D> GetPreview() const;
	inline const bool IsRendered() const { return myIsRendered; }

private:
	Volt::AssetHandle myAssetHandle;
	Volt::Entity myEntity;

	bool myIsRendered = false;

	Ref<Volt::Camera> myCamera;
	Ref<Volt::Scene> myScene;
	Ref<Volt::SceneRenderer> mySceneRenderer;
};