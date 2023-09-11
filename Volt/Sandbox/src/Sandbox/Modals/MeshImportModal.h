#pragma once

#include "Sandbox/Modals/Modal.h"

#include <Volt/Asset/Importers/FbxUtilities.h>

class MeshImportModal : public Modal
{
public:
	MeshImportModal(const std::string& strId);
	~MeshImportModal() override = default;

	void SetImportMeshes(const std::vector<std::filesystem::path>& filePaths);
	void Clear();

protected:
	void DrawModalContent() override;
	void OnOpen() override;
	void OnClose() override;

private:
	struct ImportOptions
	{
		bool isSkeletalMesh = false;
		bool combineMeshes = false;
		bool importVertexColors = false;

		glm::vec3 translation = 0.f;
		glm::vec3 rotation = 0.f;
		glm::vec3 scale = 1.f;

		bool convertScene = true;

		bool importMaterial = true;
	};

	enum class ImportType
	{
		StaticMesh,
		SkeletalMesh,
		Animation
	};

	const std::string GetStringFromImportType(const ImportType importType);
	void GetInformationOfCurrentMesh();

	ImportType m_currentImportType = ImportType::StaticMesh;
	ImportOptions m_importOptions{};
	Volt::FbxInformation m_fbxFileInformation;

	std::vector<std::filesystem::path> m_importFilePaths;
};
