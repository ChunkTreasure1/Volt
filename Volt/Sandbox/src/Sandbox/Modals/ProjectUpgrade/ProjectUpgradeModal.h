#pragma once

#include "Sandbox/Modals/Modal.h"

#include <Volt/Scene/EntityID.h>
#include <Volt/Asset/Asset.h>

#include <typeindex>

namespace Volt
{
	class IComponentTypeDesc;
	struct ComponentMember;

	class Scene;
	struct SceneLayer;
	class YAMLFileStreamReader;
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

	void ConvertAnimationGraphsToV0_1_2();

	void ConvertPrefabsToV013();
	void ConvertScenesToV013();
	void ConvertPreV013Prefab(const std::filesystem::path& filePath);

	void ConvertAssetsToV015();

	void DeserializePreV113SceneLayer(Ref<Volt::Scene> scene, Volt::SceneLayer& sceneLayer, const std::filesystem::path& layerPath, std::map<Volt::EntityID, Volt::EntityID>& entityRemapping);
	void DeserializePreV013Entity(Ref<Volt::Scene> scene, Volt::YAMLFileStreamReader& streamReader, std::map<Volt::EntityID, Volt::EntityID>& entityRemapping, bool isPrefabEntity);
	void DeserializePreV113Component(uint8_t* componentData, const Volt::IComponentTypeDesc* componentDesc, Volt::YAMLFileStreamReader& streamReader);
	void DeserializePreV113MonoScripts(Ref<Volt::Scene> scene, const Volt::EntityID entityId, Volt::YAMLFileStreamReader& streamReader);
	
	void HandleEntityRemapping(Ref<Volt::Scene> scene, const std::map<Volt::EntityID, Volt::EntityID>& entityRemapping);
	void HandleEntityArrayRemapping(Ref<Volt::Scene> scene, const std::map<Volt::EntityID, Volt::EntityID>& entityRemapping, const Volt::ComponentMember& componentMember, uint8_t* componentData);
	
	void ValidateSceneConversion(Ref<Volt::Scene> scene);
	void ValidateSceneConversionArray(Ref<Volt::Scene> scene, const Volt::ComponentMember& componentMember, uint8_t* componentData);

	const bool IsPreV013EntityNull(Volt::EntityID entityId);
	void ValidateEntityValidity(Volt::EntityID* entityId);
	const Volt::ComponentMember* TryGetComponentMemberFromName(const std::string& memberName, const Volt::IComponentTypeDesc* componentDesc);

	std::pair<std::filesystem::path, Volt::AssetHandle> DeserializeV0MetaFile(const std::filesystem::path& metaPath);
};
