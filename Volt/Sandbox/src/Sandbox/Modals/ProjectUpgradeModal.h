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
}

class YAMLStreamReader;

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
	void ConvertMetaFilesFromV0();
	void ConvertAnimationGraphsToV0_1_2();
	void ConvertMetaFilesToV011();

	void ConvertPrefabsToV113();
	void ConvertScenesToV113();

	void ConvertPreV113Prefab(const std::filesystem::path& filePath);
	
	void DeserializePreV113SceneLayer(Ref<Volt::Scene> scene, Volt::SceneLayer& sceneLayer, const std::filesystem::path& layerPath, std::map<Volt::EntityID, Volt::EntityID>& entityRemapping);
	void DeserializePreV113Entity(Ref<Volt::Scene> scene, YAMLStreamReader& streamReader, std::map<Volt::EntityID, Volt::EntityID>& entityRemapping, bool isPrefabEntity);
	void DeserializePreV113Component(uint8_t* componentData, const Volt::IComponentTypeDesc* componentDesc, YAMLStreamReader& streamReader);
	void DeserializePreV113MonoScripts(Ref<Volt::Scene> scene, const Volt::EntityID entityId, YAMLStreamReader& streamReader);
	
	void HandleEntityRemapping(Ref<Volt::Scene> scene, const std::map<Volt::EntityID, Volt::EntityID>& entityRemapping);
	void HandleEntityArrayRemapping(Ref<Volt::Scene> scene, const std::map<Volt::EntityID, Volt::EntityID>& entityRemapping, const Volt::ComponentMember& componentMember, uint8_t* componentData);
	
	void ValidateSceneConversion(Ref<Volt::Scene> scene);
	void ValidateSceneConversionArray(Ref<Volt::Scene> scene, const Volt::ComponentMember& componentMember, uint8_t* componentData);

	const bool IsPreV113EntityNull(Volt::EntityID entityId);
	void ValidateEntityValidity(Volt::EntityID* entityId);
	const Volt::ComponentMember* TryGetComponentMemberFromName(const std::string& memberName, const Volt::IComponentTypeDesc* componentDesc);

	std::pair<std::filesystem::path, Volt::AssetHandle> DeserializeV0MetaFile(const std::filesystem::path& metaPath);
};
