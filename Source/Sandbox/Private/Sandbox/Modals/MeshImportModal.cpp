#include "sbpch.h"
#include "Modals/MeshImportModal.h"

#include "Sandbox/Utility/Theme.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Utility/UIUtility.h>

#include <Volt/Project/ProjectManager.h>

#include <Volt/Asset/Mesh/MeshSource.h>
#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/Rendering/Material.h>
#include <Volt/Asset/Animation/Skeleton.h>

#include <Volt/Asset/Importers/MeshTypeImporter.h>

#include <imgui.h>

namespace Utility
{
	std::filesystem::path GetNonExistingFilePath(const std::filesystem::path& directory, const std::string& fileName)
	{
		std::filesystem::path filePath = Volt::AssetManager::GetFilesystemPath(directory / (fileName + ".vtasset"));
		uint32_t counter = 0;

		while (std::filesystem::exists(filePath))
		{
			filePath = Volt::AssetManager::GetFilesystemPath(directory / (fileName + "_" + std::to_string(counter) + ".vtasset"));
			counter++;
		}
		
 		return Volt::AssetManager::GetRelativePath(filePath);
	}
}

MeshImportModal::MeshImportModal(const std::string& strId)
	: Modal(strId)
{
}

void MeshImportModal::SetImportMeshes(const Vector<std::filesystem::path>& filePaths)
{
	m_importFilePaths = filePaths;
}

void MeshImportModal::Clear()
{
	m_currentImportType = ImportType::StaticMesh;
	m_importOptions = {};
	m_fbxFileInformation = {};
	m_importFilePaths.clear();
}

void MeshImportModal::DrawModalContent()
{
	UI::ScopedStyleFloat indentSpacing{ ImGuiStyleVar_IndentSpacing, { 0.f }};

	ImGui::Text("Importing %s", GetStringFromImportType(m_currentImportType).c_str());

	if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (UI::BeginProperties("meshOptions"))
		{
			if (m_currentImportType != ImportType::Animation)
			{
				if (UI::Property("Skeletal Mesh", m_importOptions.isSkeletalMesh))
				{
					m_currentImportType = m_importOptions.isSkeletalMesh ? ImportType::SkeletalMesh : ImportType::StaticMesh;
				}

				UI::Property("Combine Meshes", m_importOptions.combineMeshes);
				UI::Property("Import Vertex Colors", m_importOptions.importVertexColors);
			}
			else
			{
				EditorUtils::Property("Target Skeleton", m_importOptions.targetSkeleton, AssetTypes::Skeleton);
			}

			UI::EndProperties();
		}
	}

	if (m_currentImportType == ImportType::Animation || m_currentImportType == ImportType::SkeletalMesh)
	{
		if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (UI::BeginProperties("animationsOptions"))
			{
				UI::Property("Import Animations", m_importOptions.importAnimations);
				UI::EndProperties();
			}
		}
	}

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (UI::BeginProperties("options"))
		{
			UI::PropertyAxisColor("Import Translation", m_importOptions.translation, 0.f);
			UI::PropertyAxisColor("Import Rotation", m_importOptions.rotation, 0.f);
			UI::PropertyAxisColor("Import Scale", m_importOptions.scale, 1.f);

			UI::EndProperties();
		}
	}

	if (ImGui::CollapsingHeader("Miscellaneous", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (UI::BeginProperties("miscOptions"))
		{
			UI::Property("Convert Scene", m_importOptions.convertScene);

			UI::EndProperties();
		}
	}

	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (UI::BeginProperties("materialOptions"))
		{
			UI::Property("Import Material", m_importOptions.importMaterial);
			UI::EndProperties();
		}
	}

	if (ImGui::CollapsingHeader("Fbx File Information", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (UI::BeginProperties("fbxInformation"))
		{
			UI::PropertyInfoString("File Version", m_fbxFileInformation.fileVersion);
			UI::PropertyInfoString("File Creator", m_fbxFileInformation.fileCreator);
			UI::PropertyInfoString("File Creator Application", m_fbxFileInformation.fileCreatorApplication);
			UI::PropertyInfoString("File Units", m_fbxFileInformation.fileUnits);
			UI::PropertyInfoString("File Axis Direction", m_fbxFileInformation.fileAxisDirection);
			UI::EndProperties();
		}
	}

	{
		UI::ScopedButtonColor importAllColor{ EditorTheme::Buttons::BlueButton };
		if (ImGui::Button("Import All"))
		{
			for (const auto& path : m_importFilePaths)
			{
				Import(path);
			}

			Close();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Import"))
	{
		Import(m_importFilePaths.front());
		m_importFilePaths.erase(m_importFilePaths.begin());

		if (m_importFilePaths.empty())
		{
			Close();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel"))
	{
		Close();
	}
}

void MeshImportModal::OnOpen()
{
	if (m_importFilePaths.empty())
	{
		return;
	}

	GetInformationOfCurrentMesh();
}

void MeshImportModal::OnClose()
{
	Clear();
}

void MeshImportModal::GetInformationOfCurrentMesh()
{
	const auto firstFilePath = m_importFilePaths.front();
	const auto fbxInfo = Volt::FbxUtilities::GetFbxInformation(Volt::ProjectManager::GetProjectDirectory() / firstFilePath);

	m_fbxFileInformation = fbxInfo;

	if (fbxInfo.hasMesh && !fbxInfo.hasSkeleton && !fbxInfo.hasAnimation)
	{
		m_currentImportType = ImportType::StaticMesh;
	}
	else if (fbxInfo.hasMesh && fbxInfo.hasSkeleton)
	{
		m_currentImportType = ImportType::SkeletalMesh;
		m_importOptions.isSkeletalMesh = true;
	}
	else if (!fbxInfo.hasMesh && fbxInfo.hasAnimation)
	{
		m_currentImportType = ImportType::Animation;
	}
	else
	{
		m_currentImportType = ImportType::SkeletalMesh;
		m_importOptions.isSkeletalMesh = true;
	}
}

void MeshImportModal::Import(const std::filesystem::path& importPath)
{
	const std::filesystem::path destinationDirectory = importPath.parent_path();
	const std::string destinationFileName = importPath.stem().string();

	if (m_currentImportType == ImportType::StaticMesh || m_currentImportType == ImportType::SkeletalMesh)
	{
		Ref<Volt::MeshSource> importedMesh = Volt::AssetManager::GetAsset<Volt::MeshSource>(importPath);
		if (importedMesh && importedMesh->IsValid())
		{
			if (m_importOptions.importMaterial)
			{
				for (const auto& mathandle : importedMesh->GetUnderlyingMesh()->GetMaterialTable())
				{
					const auto materialAsset = Volt::AssetManager::GetAsset<Volt::Material>(mathandle);
					Volt::AssetManager::SaveAssetAs(materialAsset, Utility::GetNonExistingFilePath(destinationDirectory, materialAsset->assetName));
				}
			}

			Volt::AssetManager::SaveAssetAs(importedMesh->GetUnderlyingMesh(), Utility::GetNonExistingFilePath(destinationDirectory, destinationFileName));
		}

		if (importedMesh)
		{
			Volt::AssetManager::Get().Unload(importedMesh->handle);
		}
	}

	if (m_currentImportType == ImportType::SkeletalMesh)
	{
		Ref<Volt::Skeleton> skeleton = CreateRef<Volt::Skeleton>();
		if (Volt::MeshTypeImporter::ImportSkeleton(Volt::ProjectManager::GetRootDirectory() / importPath, *skeleton))
		{
			Volt::AssetManager::SaveAssetAs(skeleton, Utility::GetNonExistingFilePath(destinationDirectory, destinationFileName + "_Skeleton"));
		}

		if (m_importOptions.importAnimations && skeleton && m_fbxFileInformation.hasAnimation)
		{
			Ref<Volt::Animation> animation = CreateRef<Volt::Animation>();
			if (Volt::MeshTypeImporter::ImportAnimation(Volt::ProjectManager::GetRootDirectory() / importPath, skeleton, *animation))
			{
				Volt::AssetManager::SaveAssetAs(animation, Utility::GetNonExistingFilePath(destinationDirectory, destinationFileName + "_Animation"));
			}
		}
	}
	else if (m_currentImportType == ImportType::Animation)
	{
		Ref<Volt::Skeleton> targetSkeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(m_importOptions.targetSkeleton);
		if (targetSkeleton && targetSkeleton->IsValid())
		{
			Ref<Volt::Animation> animation = CreateRef<Volt::Animation>();
			if (Volt::MeshTypeImporter::ImportAnimation(Volt::ProjectManager::GetRootDirectory() / importPath, targetSkeleton, *animation))
			{
				Volt::AssetManager::SaveAssetAs(animation, Utility::GetNonExistingFilePath(destinationDirectory, destinationFileName + "_Animation"));
			}
		}
	}

}

const std::string MeshImportModal::GetStringFromImportType(const ImportType importType)
{
	switch (importType)
	{
		case ImportType::StaticMesh:
			return "Static Mesh";
			break;

		case ImportType::SkeletalMesh:
			return "Skeletal Mesh";
			break;

		case ImportType::Animation:
			return "Animation";
			break;
	}

	return "Empty";
}
