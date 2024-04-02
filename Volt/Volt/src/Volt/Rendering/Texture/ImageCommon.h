#pragma once

#include "Volt/Rendering/RendererCommon.h"

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

		RG32UI,

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

		BC6H_SF16,
		BC6H_UF16,

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

	enum class CompareOperator : uint32_t
	{
		None = 0,
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		GreaterEqual,
		Always
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
		ZeroSrcColor,
		OneMinusSrcColor,
		OneMinusSrcAlpha,
	};

	enum class ImageDimension : uint32_t
	{
		Dim1D,
		Dim2D,
		Dim3D,
		DimCube
	};

	enum class ClearMode : uint32_t
	{
		Clear = 0,
		Load,
		DontCare
	};

	struct ImageSpecification
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t depth = 0;
		uint32_t layers = 1;
		uint32_t mips = 1;

		ImageFormat format = ImageFormat::RGBA;
		ImageUsage usage = ImageUsage::Texture;
		TextureWrap wrap = TextureWrap::Repeat;
		TextureFilter filter = TextureFilter::Linear;

		MemoryUsage memoryUsage = MemoryUsage::GPUOnly;

		AnisotopyLevel anisoLevel = AnisotopyLevel::None;
		std::string debugName;

		bool isCubeMap = false;
		bool generateMips = true;
		bool mappable = false;
	};
}
