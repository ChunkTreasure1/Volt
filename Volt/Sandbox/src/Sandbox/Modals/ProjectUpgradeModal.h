#pragma once

#include "Sandbox/Modals/Modal.h"

#include <Volt/Asset/Asset.h>

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
	std::pair<std::filesystem::path, Volt::AssetHandle> DeserializeV0MetaFile(const std::filesystem::path& metaPath);
};
