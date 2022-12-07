#pragma once

#include <cstdint>

namespace Volt
{
	enum class ImageFormat : int32_t
	{
		None = 0,
		R32F,
		R16F,
		R32SI,
		R32UI,
		R8U,
		RGBA,
		RGBAS,
		RGBA16F,
		RGBA32F,
		RG16F,
		RG32F,

		SRGB,

		BC1,
		BC1SRGB,

		BC2,
		BC2SRGB,

		BC3,
		BC3SRGB,

		BC4,

		BC5,

		BC7,
		BC7SRGB,

		R32Typeless,
		R16Typeless,

		DEPTH32F,
		DEPTH16U,
		DEPTH24STENCIL8
	};

	enum class AnisotopyLevel : uint32_t
	{
		None = 0,
		X2 = 2,
		X4 = 4,
		X8 = 8,
		X16 = 16
	};

	enum class ImageUsage : uint32_t
	{
		None = 0,
		Texture,
		Attachment,
		AttachmentStorage,
		Storage
	};

	enum class TextureWrap : uint32_t
	{
		None = 0,
		Clamp,
		Repeat
	};

	enum class TextureFilter : uint32_t
	{
		None = 0,
		Linear,
		Nearest,
		Anisotopy
	};

	enum class TextureBlend : uint32_t
	{
		None,
		Alpha,
		Add,
		ZeroSrcColor
	};

	enum class ImageDimension : uint32_t
	{
		Dim1D,
		Dim2D,
		Dim3D,
		DimCube
	};

	struct ImageSpecification
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t mips = 1;
		uint32_t layers = 1;
		uint32_t mipSlice = 0;

		ImageFormat format = ImageFormat::RGBA;
		ImageFormat typelessTargetFormat = ImageFormat::R32F;
		ImageFormat typelessViewFormat = ImageFormat::DEPTH32F;
		ImageUsage usage = ImageUsage::Texture;
		TextureWrap wrap = TextureWrap::Repeat;
		TextureFilter filter = TextureFilter::Linear;

		AnisotopyLevel anisoLevel = AnisotopyLevel::None;
		std::string debugName;

		bool readable = false;
		bool writeable = false;
		bool isCubeMap = false;
	};
}