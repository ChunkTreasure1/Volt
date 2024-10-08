#pragma once

#include "Sandbox/Modals/Modal.h"

class TextureImportModal final : public Modal
{
public:
	TextureImportModal(const std::string& strId);
	~TextureImportModal() override = default;

	VT_INLINE void SetImportTextures(const Vector<std::filesystem::path>& filePaths) { m_importFilePaths = filePaths; }

protected:
	void DrawModalContent() override;
	void OnOpen() override;
	void OnClose() override;

private:
	struct ImportOptions
	{
		bool importMipMaps = true;
		bool generateMipMaps = true;
	} m_importOptions;

	std::string GetImportTypeStringFromFilepath(const std::filesystem::path& filepath);

	void Import(const std::filesystem::path filepath);
	void Clear();

	Vector<std::filesystem::path> m_importFilePaths;
};
