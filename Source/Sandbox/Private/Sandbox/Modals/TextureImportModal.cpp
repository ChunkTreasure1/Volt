#include "sbpch.h"
#include "Sandbox/Modals/TextureImportModal.h"

#include "Sandbox/Utility/Theme.h"

#include <Volt/Asset/SourceAssetImporters/FbxSourceImporter.h>

#include <Volt/Utility/UIUtility.h>

#include <CoreUtilities/StringUtility.h>


TextureImportModal::TextureImportModal(const std::string& strId)
	: Modal(strId)
{
}

void TextureImportModal::DrawModalContent()
{
	VT_ENSURE(!m_importFilePaths.empty());

	UI::ScopedStyleFloat indentSpacing{ ImGuiStyleVar_IndentSpacing, { 0.f } };

	ImGui::Text("Importing %s", GetImportTypeStringFromFilepath(m_importFilePaths.back()).c_str());

	if (ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (UI::BeginProperties("textureOptions"))
		{
			UI::Property("Import Mip Maps", m_importOptions.importMipMaps);
			UI::Property("Generate Mip Maps", m_importOptions.generateMipMaps, "If import mip maps is enabled, but none were found, mip maps will be generated");

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

void TextureImportModal::OnOpen()
{
	if (m_importFilePaths.empty())
	{
		return;
	}
}

void TextureImportModal::OnClose()
{
	Clear();
}

std::string TextureImportModal::GetImportTypeStringFromFilepath(const std::filesystem::path& filepath)
{
	std::string extension = filepath.extension().string();
	extension.erase(std::remove(extension.begin(), extension.end(), '.'));

	return Utility::ToUpper(extension);
}

void TextureImportModal::Import(const std::filesystem::path filepath)
{
	const std::filesystem::path destinationDirectory = filepath.parent_path();
	const std::string destinationFileName = filepath.stem().string();

	Volt::TextureSourceImportConfig importConfig;
	importConfig.destinationDirectory = destinationDirectory;
	importConfig.destinationFilename = destinationFileName;
	importConfig.generateMipMaps = m_importOptions.generateMipMaps;
	importConfig.importMipMaps = m_importOptions.importMipMaps;

	Volt::SourceAssetManager::ImportSourceAsset(filepath, importConfig);
}

void TextureImportModal::Clear()
{
	m_importOptions = {};
	m_importFilePaths.clear();
}
