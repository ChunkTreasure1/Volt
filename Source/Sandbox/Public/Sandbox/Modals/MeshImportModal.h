#pragma once

#include "Sandbox/Modals/Modal.h"

#include <Volt/Asset/Importers/FbxUtilities.h>
#include <AssetSystem/Asset.h>

#include <CoreUtilities/Containers/Vector.h>

class MeshImportModal final : public Modal
{
public:
	MeshImportModal(const std::string& strId);
	~MeshImportModal() override = default;

	void SetImportMeshes(const Vector<std::filesystem::path>& filePaths);

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
		Volt::AssetHandle targetSkeleton = Volt::Asset::Null();

		bool importAnimations = true;

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

	void Import(const std::filesystem::path& importPath);
	void Clear();

	ImportType m_currentImportType = ImportType::StaticMesh;
	ImportOptions m_importOptions{};
	Volt::FbxInformation m_fbxFileInformation;

	Vector<std::filesystem::path> m_importFilePaths;
};
