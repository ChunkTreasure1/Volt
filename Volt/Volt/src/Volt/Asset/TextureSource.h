#pragma once

#include <Volt/Asset/Asset.h>

namespace Volt
{
	namespace RHI
	{
		class Image2D;
	}

	class TextureSource : public Asset
	{
	public:
		~TextureSource() override = default;

		inline Ref<RHI::Image2D> GetImage() const { return m_image; }

		static AssetType GetStaticType() { return AssetType::TextureSource; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }

	private:
		friend class SourceTextureSerializer;

		Ref<RHI::Image2D> m_image;
	};
}
