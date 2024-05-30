#pragma once

#include <Volt/Asset/Asset.h>

#include <VoltRHI/Images/Image2D.h>

namespace Volt
{
	class TextureSource : public Asset
	{
	public:
		~TextureSource() override = default;

		inline RefPtr<RHI::Image2D> GetImage() const { return m_image; }

		static AssetType GetStaticType() { return AssetType::TextureSource; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }

	private:
		friend class SourceTextureSerializer;

		RefPtr<RHI::Image2D> m_image;
	};
}
