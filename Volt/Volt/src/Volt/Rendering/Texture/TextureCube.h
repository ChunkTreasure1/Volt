#pragma once

#include "Volt/Rendering/Texture/Image2D.h"

namespace Volt
{
	class TextureCube
	{
	public:
		TextureCube(ImageFormat aFormat, uint32_t aWidth, uint32_t aHeight, const void* aData);
		TextureCube() = default;
		~TextureCube();

		const uint32_t GetWidth() const;
		const uint32_t GetHeight() const;

		inline const Ref<Image2D> GetImage() const { return myImage; }

		static Ref<TextureCube> Create(ImageFormat aFormat, uint32_t aWidth, uint32_t aHeight, const void* aData = nullptr);

	private:
		Ref<Image2D> myImage;
	};
}