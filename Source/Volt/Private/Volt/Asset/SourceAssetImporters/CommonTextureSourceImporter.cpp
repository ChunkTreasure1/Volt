#include "vtpch.h"
#include "Volt/Asset/SourceAssetImporters/CommonTextureSourceImporter.h"
#include "Volt/Asset/SourceAssetImporters/ImportConfigs.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include <AssetSystem/AssetManager.h>

#include <RHIModule/Images/Image.h>
#include <RHIModule/Images/ImageUtility.h>

#include <stb/stb_image.h>

VT_DEFINE_LOG_CATEGORY(LogCommonTextureSourceImporter);

namespace Volt
{
	Vector<Ref<Asset>> CommonTextureSourceImporter::ImportInternal(const std::filesystem::path& filepath, const void* config, const SourceAssetUserImportData& userData) const
	{
		VT_PROFILE_FUNCTION();
		const TextureSourceImportConfig& importConfig = *reinterpret_cast<const TextureSourceImportConfig*>(config);

		int32_t width;
		int32_t height;
		int32_t channels;

		// #TODO_Ivar: Add failure checking
		stbi_set_flip_vertically_on_load(0);
		
		const bool isHDR = stbi_is_hdr(filepath.string().c_str());
		const bool is16Bit = stbi_is_16_bit(filepath.string().c_str());

		void* data = nullptr;

		if (isHDR)
		{
			data = stbi_loadf(filepath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		}
		else
		{
			data = stbi_load(filepath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		}

		if (!data)
		{
			const std::string error = std::format("Failed to import file {}. Reason: {}", filepath, stbi_failure_reason());
			VT_LOGC(Error, LogCommonTextureSourceImporter, error);
			userData.OnError(error);
			return {};
		}

		RHI::PixelFormat format = RHI::PixelFormat::R8G8B8A8_UNORM;

		if (is16Bit)
		{
			format = isHDR ? RHI::PixelFormat::R16G16B16A16_SFLOAT : RHI::PixelFormat::R16G16B16A16_UNORM;
		}

		if (!is16Bit && isHDR)
		{
			format = RHI::PixelFormat::R32G32B32A32_SFLOAT;
		}

		const uint32_t mipLevelCount = importConfig.generateMipMaps ? RHI::Utility::CalculateMipCount(width, height) : 1u;

		RHI::ImageSpecification specification{};
		specification.format = format;
		specification.usage = RHI::ImageUsage::Texture;
		specification.width = width;
		specification.height = height;
		specification.mips = mipLevelCount;
		specification.generateMips = importConfig.generateMipMaps;
		specification.debugName = importConfig.destinationFilename;

		RefPtr<RHI::Image> image = RHI::Image::Create(specification, data);

		Ref<Texture2D> voltTexture;

		if (importConfig.createAsMemoryAsset)
		{
			voltTexture = AssetManager::CreateMemoryAsset<Texture2D>(importConfig.destinationFilename);
		}
		else
		{
			voltTexture = AssetManager::CreateAsset<Texture2D>(importConfig.destinationDirectory, importConfig.destinationFilename);
		}

		voltTexture->SetImage(image);

		return { voltTexture };
	}

	SourceAssetFileInformation CommonTextureSourceImporter::GetSourceFileInformation(const std::filesystem::path& filepath) const
	{
		VT_ENSURE(false);
		return SourceAssetFileInformation();
	}
}
