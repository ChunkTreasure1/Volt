#include "vtpch.h"
#include "DDSUtility.h"

#include "Volt/Utility/DirectXUtils.h"

#include <DirectXTex/DirectXTex.h>

namespace Volt
{
	const DDSUtility::TextureData DDSUtility::GetRawDataFromDDS(const std::filesystem::path& path)
	{
		DirectX::TexMetadata metadata{};
		DirectX::ScratchImage scratchImage{};

		VT_DX_CHECK(DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage));

		const DirectX::Image* compressedImage = scratchImage.GetImage(0, 0, 0);
		DirectX::ScratchImage decompressedImage;
		VT_DX_CHECK(DirectX::Decompress(*compressedImage, DXGI_FORMAT_R8G8B8A8_UNORM, decompressedImage));

		TextureData result{};

		result.dataBuffer.Allocate(decompressedImage.GetPixelsSize());
		result.dataBuffer.Copy(decompressedImage.GetPixels(), decompressedImage.GetPixelsSize());
		result.width = (uint32_t)metadata.width;
		result.height = (uint32_t)metadata.height;

		return result;
	}
}
