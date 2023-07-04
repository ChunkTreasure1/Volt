#include "sbpch.h"
#include "MeshImportModal.h"

#include "Sandbox/Utility/Theme.h"

#include <Volt/Utility/UIUtility.h>

#include <imgui.h>

MeshImportModal::MeshImportModal(const std::string& strId)
	: Modal(strId)
{
}

void MeshImportModal::SetImportMeshes(const std::vector<std::filesystem::path>& filePaths)
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
	ImGui::Text("Importing %s", GetStringFromImportType(m_currentImportType).c_str());

	if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (UI::BeginProperties("meshOptions"))
		{
			UI::Property("Skeletal Mesh", m_importOptions.isSkeletalMesh);
			UI::Property("Combine Meshes", m_importOptions.combineMeshes);
			UI::Property("Import Vertex Colors", m_importOptions.importVertexColors);

			UI::EndProperties();
		}
	}

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (UI::BeginProperties("transformOptions"))
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
			Close();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Import"))
	{
		Close();
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
	}
	else if (!fbxInfo.hasMesh && fbxInfo.hasAnimation)
	{
		m_currentImportType = ImportType::Animation;
	}
	else
	{
		m_currentImportType = ImportType::SkeletalMesh;
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
