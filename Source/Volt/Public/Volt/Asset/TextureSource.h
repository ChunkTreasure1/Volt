#pragma once

#include "Volt/Asset/AssetTypes.h"

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetFactory.h>

#include <RHIModule/Images/Image.h>

namespace Volt
{
	class TextureSource : public Asset
	{
	public:
		~TextureSource() override = default;

		inline RefPtr<RHI::Image> GetImage() const { return m_image; }

		static AssetType GetStaticType() { return AssetTypes::TextureSource; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }

	private:
		friend class SourceTextureSerializer;

		RefPtr<RHI::Image> m_image;
	};
}
