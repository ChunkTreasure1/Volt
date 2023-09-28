#pragma once

#include "Sandbox/Modals/Modal.h"

#include <Volt/Asset/Asset.h>
#include <entt.hpp>

#include <typeindex>

namespace Volt
{
	class IComponentTypeDesc;
	struct ComponentMember;

	class Scene;
	struct SceneLayer;
	class YAMLStreamReader;
}

class ProjectUpgradeModal : public Modal
{
public:
	ProjectUpgradeModal(const std::string& strId);
	~ProjectUpgradeModal() override = default;

protected:
	void DrawModalContent() override;
	void OnOpen() override;
	void OnClose() override;

private:
	void UpgradeCurrentProject();
	void ConvertMetaFilesToV011();

	void ConvertPrefabsToV113();
	void ConvertScenesToV113();

	void DeserializePreV113SceneLayer(Ref<Volt::Scene> scene, Volt::SceneLayer& sceneLayer, const std::filesystem::path& layerPath);
	void DeserializePreV113Entity(Ref<Volt::Scene> scene, Volt::YAMLStreamReader& streamReader);
	void DeserializePreV113Component(uint8_t* componentData, const Volt::IComponentTypeDesc* componentDesc, Volt::YAMLStreamReader& streamReader);

	const bool IsPreV113EntityNull(entt::entity entityId);
	const Volt::ComponentMember* TryGetComponentMemberFromName(const std::string& memberName, const Volt::IComponentTypeDesc* componentDesc);

	std::pair<std::filesystem::path, Volt::AssetHandle> DeserializeV0MetaFile(const std::filesystem::path& metaPath);
};
