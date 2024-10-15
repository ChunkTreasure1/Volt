#include "sbpch.h"
#include "Modals/MeshImportModal.h"

#include "Sandbox/Utility/Theme.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Utility/UIUtility.h>

#include <Volt/Asset/SourceAssetImporters/FbxSourceImporter.h>

#include <imgui.h>

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
	m_fileInformation = {};
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

	if (ImGui::CollapsingHeader("File Information", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (UI::BeginProperties("fileInformation"))
		{
			UI::PropertyInfoString("File Version", m_fileInformation.fileVersion);
			UI::PropertyInfoString("File Creator", m_fileInformation.fileCreator);
			UI::PropertyInfoString("File Creator Application", m_fileInformation.fileCreatorApplication);
			UI::PropertyInfoString("File Units", m_fileInformation.fileUnits);
			UI::PropertyInfoString("File Axis Direction", m_fileInformation.fileAxisDirection);
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
	m_fileInformation = Volt::SourceAssetManager::GetSourceAssetFileInformation(firstFilePath);
	
	if (m_fileInformation.hasMesh && !m_fileInformation.hasSkeleton && !m_fileInformation.hasAnimation)
	{
		m_currentImportType = ImportType::StaticMesh;
	}
	else if (m_fileInformation.hasMesh && m_fileInformation.hasSkeleton)
	{
		m_currentImportType = ImportType::SkeletalMesh;
		m_importOptions.isSkeletalMesh = true;
	}
	else if (!m_fileInformation.hasMesh && m_fileInformation.hasAnimation)
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

	Volt::MeshSourceImportConfig importConfig;
	importConfig.destinationDirectory = destinationDirectory;
	importConfig.destinationFilename = destinationFileName;

	switch (m_currentImportType)
	{
		case MeshImportModal::ImportType::StaticMesh: importConfig.importType = Volt::MeshSourceImportType::StaticMesh; break;
		case MeshImportModal::ImportType::SkeletalMesh: importConfig.importType = Volt::MeshSourceImportType::SkeletalMesh; break;
		case MeshImportModal::ImportType::Animation: importConfig.importType = Volt::MeshSourceImportType::Animation; break;
	}

	importConfig.translation = m_importOptions.translation;
	importConfig.rotation = m_importOptions.rotation;
	importConfig.scale = m_importOptions.scale;
	importConfig.importAnimationIfSkeletalMesh = m_importOptions.importAnimations;
	importConfig.importVertexColors = m_importOptions.importVertexColors;
	importConfig.importMaterial = m_importOptions.importMaterial;
	importConfig.convertScene = m_importOptions.convertScene;
	importConfig.combineMeshes = m_importOptions.combineMeshes;
	importConfig.removeDegeneratePolygons = true;
	importConfig.triangulate = true;
	importConfig.generateTangents = true;
	importConfig.generateNormalsMode = Volt::GenerateNormalsMode::Smooth;

	Volt::SourceAssetManager::ImportSourceAsset(importPath, importConfig);
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
