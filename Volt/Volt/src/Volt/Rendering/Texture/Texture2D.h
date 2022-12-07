#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Rendering/Texture/Image2D.h"

namespace Volt
{
	class Texture2D : public Asset
	{
	public:
		Texture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data);
		Texture2D(Ref<Image2D> image);
		Texture2D() = default;
		~Texture2D() override;

		const uint32_t GetWidth() const;
		const uint32_t GetHeight() const;

		void Bind(uint32_t aSlot) const;

		inline const Ref<Image2D> GetImage() const { return myImage; }

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		static Ref<Texture2D> Create(Ref<Image2D> image);

	private:
		friend class DDSTextureImporter;
		friend class DefaultTextureImporter;

		Ref<Image2D> myImage;
	};
}