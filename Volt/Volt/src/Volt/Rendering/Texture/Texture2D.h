#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Rendering/Texture/ImageCommon.h"

namespace Volt
{
	class Image2D;
	class Texture2D : public Asset
	{
	public:
		Texture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data);
		Texture2D(Ref<Image2D> image);
		Texture2D() = default;
		~Texture2D() override;

		const uint32_t GetWidth() const;
		const uint32_t GetHeight() const;

		inline const Ref<Image2D> GetImage() const { return myImage; }
		inline void SetImage(Ref<Image2D> image) { myImage = image; }

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }

		static Ref<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		static Ref<Texture2D> Create(Ref<Image2D> image);

	private:
		friend class DDSTextureImporter;
		friend class DefaultTextureImporter;
		friend class TextureSerializer;

		Ref<Image2D> myImage;
	};
}
