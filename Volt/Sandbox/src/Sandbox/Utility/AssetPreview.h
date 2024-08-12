#pragma once

#include <Volt/Asset/Asset.h>
#include <Volt/Scene/Entity.h>

namespace Volt
{
	namespace RHI
	{
		class Image2D;
	}

	class Scene;
	class SceneRenderer;
	class Camera;
}

class AssetPreview
{
public:
	AssetPreview(const std::filesystem::path& path);

	void Render();
	const RefPtr<Volt::RHI::Image> GetPreview() const;
	inline const bool IsRendered() const { return m_isRenderered; }

private:
	Volt::AssetHandle m_assetHandle;
	Volt::Entity m_entity;

	bool m_isRenderered = false;

	Ref<Volt::Camera> m_camera;
	Ref<Volt::Scene> m_scene;
	Ref<Volt::SceneRenderer> m_sceneRenderer;
};
