#pragma once
#include "VoltRHI/Core/Core.h"

#include <CoreUtilities/Containers/StackVector.h>
#include <CoreUtilities/Variant.h>
#include <CoreUtilities/Pointers/WeakPtr.h>

#include <array>
#include <functional>
#include <variant>

namespace Volt::RHI
{
	class PhysicalGraphicsDevice;
	class GraphicsDevice;
	class ImageView;
	class RHIResource;

	inline static constexpr size_t MAX_COLOR_ATTACHMENT_COUNT = 8;
	inline static constexpr size_t MAX_ATTACHMENT_COUNT = MAX_COLOR_ATTACHMENT_COUNT + 1;

	enum class QueueType
	{
		Graphics,
		Compute,
		TransferCopy
	};

	enum class GraphicsAPI
	{
		Vulkan,
		D3D12,
		MoltenVk,
		Mock,
	};

	enum class DeviceVendor
	{
		AMD,
		NVIDIA,
		Intel,

		Unknown
	};

	enum class LogSeverity
	{
		Trace,
		Info,
		Warning,
		Error,
	};

	enum class PresentMode : uint32_t
	{
		Immediate = 0, // duh
		Mailbox = 1, // 
		FIFO = 2, // V-sync
		FIFORelaxed = 3 // V-sync if slow dont
	};

	enum class PixelFormat : uint32_t
	{
		UNDEFINED = 0,
		R4G4_UNORM_PACK8 = 1,
		R4G4B4A4_UNORM_PACK16 = 2,
		B4G4R4A4_UNORM_PACK16 = 3,
		R5G6B5_UNORM_PACK16 = 4,
		B5G6R5_UNORM_PACK16 = 5,
		R5G5B5A1_UNORM_PACK16 = 6,
		B5G5R5A1_UNORM_PACK16 = 7,
		A1R5G5B5_UNORM_PACK16 = 8,
		R8_UNORM = 9,
		R8_SNORM = 10,
		R8_USCALED = 11,
		R8_SSCALED = 12,
		R8_UINT = 13,
		R8_SINT = 14,
		R8_SRGB = 15,
		R8G8_UNORM = 16,
		R8G8_SNORM = 17,
		R8G8_USCALED = 18,
		R8G8_SSCALED = 19,
		R8G8_UINT = 20,
		R8G8_SINT = 21,
		R8G8_SRGB = 22,
		R8G8B8_UNORM = 23,
		R8G8B8_SNORM = 24,
		R8G8B8_USCALED = 25,
		R8G8B8_SSCALED = 26,
		R8G8B8_UINT = 27,
		R8G8B8_SINT = 28,
		R8G8B8_SRGB = 29,
		B8G8R8_UNORM = 30,
		B8G8R8_SNORM = 31,
		B8G8R8_USCALED = 32,
		B8G8R8_SSCALED = 33,
		B8G8R8_UINT = 34,
		B8G8R8_SINT = 35,
		B8G8R8_SRGB = 36,
		R8G8B8A8_UNORM = 37,
		R8G8B8A8_SNORM = 38,
		R8G8B8A8_USCALED = 39,
		R8G8B8A8_SSCALED = 40,
		R8G8B8A8_UINT = 41,
		R8G8B8A8_SINT = 42,
		R8G8B8A8_SRGB = 43,
		B8G8R8A8_UNORM = 44,
		B8G8R8A8_SNORM = 45,
		B8G8R8A8_USCALED = 46,
		B8G8R8A8_SSCALED = 47,
		B8G8R8A8_UINT = 48,
		B8G8R8A8_SINT = 49,
		B8G8R8A8_SRGB = 50,
		A8B8G8R8_UNORM_PACK32 = 51,
		A8B8G8R8_SNORM_PACK32 = 52,
		A8B8G8R8_USCALED_PACK32 = 53,
		A8B8G8R8_SSCALED_PACK32 = 54,
		A8B8G8R8_UINT_PACK32 = 55,
		A8B8G8R8_SINT_PACK32 = 56,
		A8B8G8R8_SRGB_PACK32 = 57,
		A2R10G10B10_UNORM_PACK32 = 58,
		A2R10G10B10_SNORM_PACK32 = 59,
		A2R10G10B10_USCALED_PACK32 = 60,
		A2R10G10B10_SSCALED_PACK32 = 61,
		A2R10G10B10_UINT_PACK32 = 62,
		A2R10G10B10_SINT_PACK32 = 63,
		A2B10G10R10_UNORM_PACK32 = 64,
		A2B10G10R10_SNORM_PACK32 = 65,
		A2B10G10R10_USCALED_PACK32 = 66,
		A2B10G10R10_SSCALED_PACK32 = 67,
		A2B10G10R10_UINT_PACK32 = 68,
		A2B10G10R10_SINT_PACK32 = 69,
		R16_UNORM = 70,
		R16_SNORM = 71,
		R16_USCALED = 72,
		R16_SSCALED = 73,
		R16_UINT = 74,
		R16_SINT = 75,
		R16_SFLOAT = 76,
		R16G16_UNORM = 77,
		R16G16_SNORM = 78,
		R16G16_USCALED = 79,
		R16G16_SSCALED = 80,
		R16G16_UINT = 81,
		R16G16_SINT = 82,
		R16G16_SFLOAT = 83,
		R16G16B16_UNORM = 84,
		R16G16B16_SNORM = 85,
		R16G16B16_USCALED = 86,
		R16G16B16_SSCALED = 87,
		R16G16B16_UINT = 88,
		R16G16B16_SINT = 89,
		R16G16B16_SFLOAT = 90,
		R16G16B16A16_UNORM = 91,
		R16G16B16A16_SNORM = 92,
		R16G16B16A16_USCALED = 93,
		R16G16B16A16_SSCALED = 94,
		R16G16B16A16_UINT = 95,
		R16G16B16A16_SINT = 96,
		R16G16B16A16_SFLOAT = 97,
		R32_UINT = 98,
		R32_SINT = 99,
		R32_SFLOAT = 100,
		R32G32_UINT = 101,
		R32G32_SINT = 102,
		R32G32_SFLOAT = 103,
		R32G32B32_UINT = 104,
		R32G32B32_SINT = 105,
		R32G32B32_SFLOAT = 106,
		R32G32B32A32_UINT = 107,
		R32G32B32A32_SINT = 108,
		R32G32B32A32_SFLOAT = 109,
		R64_UINT = 110,
		R64_SINT = 111,
		R64_SFLOAT = 112,
		R64G64_UINT = 113,
		R64G64_SINT = 114,
		R64G64_SFLOAT = 115,
		R64G64B64_UINT = 116,
		R64G64B64_SINT = 117,
		R64G64B64_SFLOAT = 118,
		R64G64B64A64_UINT = 119,
		R64G64B64A64_SINT = 120,
		R64G64B64A64_SFLOAT = 121,
		B10G11R11_UFLOAT_PACK32 = 122,
		E5B9G9R9_UFLOAT_PACK32 = 123,
		D16_UNORM = 124,
		X8_D24_UNORM_PACK32 = 125,
		D32_SFLOAT = 126,
		S8_UINT = 127,
		D16_UNORM_S8_UINT = 128,
		D24_UNORM_S8_UINT = 129,
		D32_SFLOAT_S8_UINT = 130,
		BC1_RGB_UNORM_BLOCK = 131,
		BC1_RGB_SRGB_BLOCK = 132,
		BC1_RGBA_UNORM_BLOCK = 133,
		BC1_RGBA_SRGB_BLOCK = 134,
		BC2_UNORM_BLOCK = 135,
		BC2_SRGB_BLOCK = 136,
		BC3_UNORM_BLOCK = 137,
		BC3_SRGB_BLOCK = 138,
		BC4_UNORM_BLOCK = 139,
		BC4_SNORM_BLOCK = 140,
		BC5_UNORM_BLOCK = 141,
		BC5_SNORM_BLOCK = 142,
		BC6H_UFLOAT_BLOCK = 143,
		BC6H_SFLOAT_BLOCK = 144,
		BC7_UNORM_BLOCK = 145,
		BC7_SRGB_BLOCK = 146,
		ETC2_R8G8B8_UNORM_BLOCK = 147,
		ETC2_R8G8B8_SRGB_BLOCK = 148,
		ETC2_R8G8B8A1_UNORM_BLOCK = 149,
		ETC2_R8G8B8A1_SRGB_BLOCK = 150,
		ETC2_R8G8B8A8_UNORM_BLOCK = 151,
		ETC2_R8G8B8A8_SRGB_BLOCK = 152,
		EAC_R11_UNORM_BLOCK = 153,
		EAC_R11_SNORM_BLOCK = 154,
		EAC_R11G11_UNORM_BLOCK = 155,
		EAC_R11G11_SNORM_BLOCK = 156,
		ASTC_4x4_UNORM_BLOCK = 157,
		ASTC_4x4_SRGB_BLOCK = 158,
		ASTC_5x4_UNORM_BLOCK = 159,
		ASTC_5x4_SRGB_BLOCK = 160,
		ASTC_5x5_UNORM_BLOCK = 161,
		ASTC_5x5_SRGB_BLOCK = 162,
		ASTC_6x5_UNORM_BLOCK = 163,
		ASTC_6x5_SRGB_BLOCK = 164,
		ASTC_6x6_UNORM_BLOCK = 165,
		ASTC_6x6_SRGB_BLOCK = 166,
		ASTC_8x5_UNORM_BLOCK = 167,
		ASTC_8x5_SRGB_BLOCK = 168,
		ASTC_8x6_UNORM_BLOCK = 169,
		ASTC_8x6_SRGB_BLOCK = 170,
		ASTC_8x8_UNORM_BLOCK = 171,
		ASTC_8x8_SRGB_BLOCK = 172,
		ASTC_10x5_UNORM_BLOCK = 173,
		ASTC_10x5_SRGB_BLOCK = 174,
		ASTC_10x6_UNORM_BLOCK = 175,
		ASTC_10x6_SRGB_BLOCK = 176,
		ASTC_10x8_UNORM_BLOCK = 177,
		ASTC_10x8_SRGB_BLOCK = 178,
		ASTC_10x10_UNORM_BLOCK = 179,
		ASTC_10x10_SRGB_BLOCK = 180,
		ASTC_12x10_UNORM_BLOCK = 181,
		ASTC_12x10_SRGB_BLOCK = 182,
		ASTC_12x12_UNORM_BLOCK = 183,
		ASTC_12x12_SRGB_BLOCK = 184,
		G8B8G8R8_422_UNORM = 1000156000,
		B8G8R8G8_422_UNORM = 1000156001,
		G8_B8_R8_3PLANE_420_UNORM = 1000156002,
		G8_B8R8_2PLANE_420_UNORM = 1000156003,
		G8_B8_R8_3PLANE_422_UNORM = 1000156004,
		G8_B8R8_2PLANE_422_UNORM = 1000156005,
		G8_B8_R8_3PLANE_444_UNORM = 1000156006,
		R10X6_UNORM_PACK16 = 1000156007,
		R10X6G10X6_UNORM_2PACK16 = 1000156008,
		R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 1000156009,
		G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 1000156010,
		B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 1000156011,
		G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
		G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 1000156013,
		G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
		G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 1000156015,
		G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
		R12X4_UNORM_PACK16 = 1000156017,
		R12X4G12X4_UNORM_2PACK16 = 1000156018,
		R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 1000156019,
		G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 1000156020,
		B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 1000156021,
		G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
		G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 1000156023,
		G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
		G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 1000156025,
		G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
		G16B16G16R16_422_UNORM = 1000156027,
		B16G16R16G16_422_UNORM = 1000156028,
		G16_B16_R16_3PLANE_420_UNORM = 1000156029,
		G16_B16R16_2PLANE_420_UNORM = 1000156030,
		G16_B16_R16_3PLANE_422_UNORM = 1000156031,
		G16_B16R16_2PLANE_422_UNORM = 1000156032,
		G16_B16_R16_3PLANE_444_UNORM = 1000156033,
		G8_B8R8_2PLANE_444_UNORM = 1000330000,
		G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 = 1000330001,
		G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 = 1000330002,
		G16_B16R16_2PLANE_444_UNORM = 1000330003,
		A4R4G4B4_UNORM_PACK16 = 1000340000,
		A4B4G4R4_UNORM_PACK16 = 1000340001,
		ASTC_4x4_SFLOAT_BLOCK = 1000066000,
		ASTC_5x4_SFLOAT_BLOCK = 1000066001,
		ASTC_5x5_SFLOAT_BLOCK = 1000066002,
		ASTC_6x5_SFLOAT_BLOCK = 1000066003,
		ASTC_6x6_SFLOAT_BLOCK = 1000066004,
		ASTC_8x5_SFLOAT_BLOCK = 1000066005,
		ASTC_8x6_SFLOAT_BLOCK = 1000066006,
		ASTC_8x8_SFLOAT_BLOCK = 1000066007,
		ASTC_10x5_SFLOAT_BLOCK = 1000066008,
		ASTC_10x6_SFLOAT_BLOCK = 1000066009,
		ASTC_10x8_SFLOAT_BLOCK = 1000066010,
		ASTC_10x10_SFLOAT_BLOCK = 1000066011,
		ASTC_12x10_SFLOAT_BLOCK = 1000066012,
		ASTC_12x12_SFLOAT_BLOCK = 1000066013,
		PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
		PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
		PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
		PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
		PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
		PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
		PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
		PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
		R16G16_S10_5_NV = 1000464000,
	};

	enum class ColorSpace : uint32_t
	{
		SRGB_NONLINEAR = 0,
		DISPLAY_P3_NONLINEAR = 1000104001,
		EXTENDED_SRGB_LINEAR = 1000104002,
		DISPLAY_P3_LINEAR = 1000104003,
		DCI_P3_NONLINEAR = 1000104004,
		BT709_LINEAR = 1000104005,
		BT709_NONLINEAR = 1000104006,
		BT2020_LINEAR = 1000104007,
		HDR10_ST2084 = 1000104008,
		DOLBYVISION = 1000104009,
		HDR10_HLG = 1000104010,
		ADOBERGB_LINEAR = 1000104011,
		ADOBERGB_NONLINEAR = 1000104012,
		PASS_THROUGH_EXT = 1000104013,
		EXTENDED_SRGB_NONLINEAR = 1000104014,
		DISPLAY_NATIVE_AMD = 1000213000,
	};

	enum class Topology : uint32_t
	{
		TriangleList = 0,
		LineList,
		TriangleStrip,
		PatchList,
		PointList
	};

	enum class CullMode : uint32_t
	{
		Front = 0,
		Back,
		FrontAndBack,
		None
	};

	enum class FillMode : uint32_t
	{
		Solid,
		Wireframe
	};

	enum class DepthMode : uint32_t
	{
		Read = 0,
		Write,
		ReadWrite,
		None
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

	enum class AnisotropyLevel : uint32_t
	{
		None = 0,
		X2 = 2,
		X4 = 4,
		X8 = 8,
		X16 = 16
	};

	enum class BufferUsage : uint32_t
	{
		None = 0,
		TransferSrc = BIT(0),
		TransferDst = BIT(1),
		UniformBuffer = BIT(2),
		StorageBuffer = BIT(3),
		IndexBuffer = BIT(4),
		VertexBuffer = BIT(5),
		IndirectBuffer = BIT(6),
	
		// Vulkan only
		DescriptorBuffer = BIT(7)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(BufferUsage);

	enum class MemoryUsage : uint32_t
	{
		None = 0,
		GPU = BIT(0),
		CPU = BIT(1),
		CPUToGPU = BIT(2),
		GPUToCPU = BIT(3),
		Dedicated = BIT(4)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(MemoryUsage);

	enum class ImageViewType : uint32_t
	{
		View1D = 0,
		View2D,
		View3D,

		View1DArray,
		View2DArray,
		
		ViewCube,
		ViewCubeArray
	};

	enum class ImageAspect : uint32_t
	{
		None = 0,
		Color = BIT(0),
		Depth = BIT(1),
		Stencil = BIT(2)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(ImageAspect);

	enum class ClearMode
	{
		Clear = 0,
		Load,
		DontCare
	};

	enum class ResourceType
	{
		Image2D = 0,
		Image3D,

		IndexBuffer,
		VertexBuffer,
		UniformBuffer,
		StorageBuffer
	};

	enum class ResourceBarrierType
	{
		Buffer,
		Image
	};

	enum class BarrierStage : uint64_t
	{
		None = 0,
		All = BIT(0),
		Draw = BIT(1), // ?
		IndexInput = BIT(2),
		VertexShader = BIT(3), // VERTEX_ATTRIBUTE_INPUT & VERTEX_SHADER
		PixelShader = BIT(4), // FRAGMENT_SHADER
		DepthStencil = BIT(5), // EARLY_FRAGMENT_TESTS & LATE_FRAGMENT_TESTS
		RenderTarget = BIT(6), // COLOR_ATTACHMENT_OUTPUT
		ComputeShader = BIT(7), // COMPUTE_SHADER
		RayTracing = BIT(8), // RAY_TRACING
		Copy = BIT(9), // COPY
		Resolve = BIT(10), // RESOLVE,
		Indirect = BIT(11), // DRAW_INDIRECT
		AllGraphics = BIT(12), // ALL_GRAPHICS
		VideoDecode = BIT(13),
		VideoEncode = BIT(14),
		BuildAccelerationStructure = BIT(15)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(BarrierStage);

	enum class BarrierAccess : uint64_t
	{
		None = 0,
		VertexBuffer = BIT(0), // VERTEX_ATTRIBUTE_READ
		UniformBuffer = BIT(1), // UNIFORM_READ
		IndexBuffer = BIT(2), // INDEX_READ
		RenderTarget = BIT(3), // COLOR_ATTACHMENT_WRITE
		ShaderWrite = BIT(4), // SHADER_WRITE
		DepthStencilWrite = BIT(5), // DEPTH_STENCIL_ATTACHMENT_WRITE
		DepthStencilRead = BIT(6), // DEPTH_STENCIL_ATTACHMENT_READ
		IndirectArgument = BIT(7), // INDIRECT_COMMAND_READ
		TransferSource = BIT(8),
		TransferDestination = BIT(9),
		ShaderRead = BIT(10),
		ResolveSource = BIT(11), // ?
		ResolveDestination = BIT(12), // ?
		AccelerationStructureRead = BIT(13), // ACCELERATION_STRUCTURE_READ
		AccelerationStructureWrite = BIT(14), // ACCELERATION_STRUCTURE_WRITEs
		VideoDecodeRead = BIT(15),
		VideoDecodeWrite = BIT(16),
		VideoEncodeRead = BIT(17),
		VideoEncodeWrite = BIT(18)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(BarrierAccess);

	enum class ImageLayout : uint64_t
	{
		Undefined = 0,
		Present = BIT(0), // PRESENT_SRC
		RenderTarget = BIT(1), // COLOR_ATTACHMENT_OPTIMAL
		ShaderWrite = BIT(2), // GENERAL
		DepthStencilWrite = BIT(3), // DEPTH_STENCIL_ATTACHMENT_WRITE
		DepthStencilRead = BIT(4), // DEPTH_STENCIL_ATTACHMENT_READ
		ShaderRead = BIT(5), // SHADER_READ_ONLY_OPTIMAL
		TransferSource = BIT(6),
		TransferDestination = BIT(7),
		ResolveSource = BIT(8),
		ResolveDestination = BIT(9),
		VideoDecodeRead = BIT(10),
		VideoDecodeWrite = BIT(11),
		VideoEncodeRead = BIT(12),
		VideoEncodeWrite = BIT(13),
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(ImageLayout);

	enum class BarrierType : uint64_t
	{
		Image = 0,
		Buffer,
		Global
	};

	// --- structures --- \\

	struct LogInfo
	{
		std::function<void(LogSeverity, std::string_view)> logCallback;
		bool enabled = false;
	};

	struct ResourceManagementInfo
	{
		std::function<void(std::function<void()>&&)> resourceDeletionCallback;
	};

	struct MemoryRequirement
	{
		uint64_t size = 0;
		uint64_t alignment = 0;
		uint32_t memoryTypeBits = 0;
	};

	struct PhysicalDeviceCreateInfo
	{
	};

	struct DeviceCapabilities
	{
		DeviceVendor deviceVendor;
		std::string_view gpuName;

		bool timestampSupport;
		float timestampPeriod = 0.f;

		float maxAnisotropyLevel = 1.f;
	};

	struct GraphicsDeviceCreateInfo
	{
		RefPtr<PhysicalGraphicsDevice> physicalDevice;
	};

	struct GraphicsContextCreateInfo
	{
		GraphicsAPI graphicsApi;
		PhysicalDeviceCreateInfo physicalDeviceInfo;
		GraphicsDeviceCreateInfo graphicsDeviceInfo;
	};

	struct DeviceQueueCreateInfo
	{
		GraphicsDevice* graphicsDevice;
		QueueType queueType;
	};

	struct ImageSpecification
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t depth = 1;
		uint32_t layers = 1;
		uint32_t mips = 1;

		PixelFormat format = PixelFormat::R8G8B8A8_UNORM;
		ImageUsage usage = ImageUsage::Texture;

		MemoryUsage memoryUsage = MemoryUsage::GPU;

		AnisotropyLevel anisoLevel = AnisotropyLevel::None;
		std::string debugName;

		bool isCubeMap = false;
		bool generateMips = false;

		bool initializeImage = true;
	};

	struct Extent2D
	{
		uint32_t width = 0;
		uint32_t height = 0;
	};

	struct Offset2D
	{
		int32_t x;
		int32_t y;
	};

	struct Extent3D
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 0;
	};

	struct Rect2D
	{
		Offset2D offset;
		Extent2D extent;
	};

	struct Viewport
	{
		float x;
		float y;

		float width;
		float height;

		float minDepth;
		float maxDepth;
	};

	struct AttachmentInfo
	{
		WeakPtr<ImageView> view;

		ClearMode clearMode;

		inline void SetClearColor(float r, float g, float b, float a) 
		{ 
			clearColor.float32[0] = r; 
			clearColor.float32[1] = g; 
			clearColor.float32[2] = b;
			clearColor.float32[3] = a;
		}

		inline void SetClearColor(int32_t r, int32_t g, int32_t b, int32_t a)
		{
			clearColor.int32[0] = r;
			clearColor.int32[1] = g;
			clearColor.int32[2] = b;
			clearColor.int32[3] = a;
		}

		inline void SetClearColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
		{
			clearColor.uint32[0] = r;
			clearColor.uint32[1] = g;
			clearColor.uint32[2] = b;
			clearColor.uint32[3] = a;
		}

		union
		{
			float float32[4];
			int32_t int32[4];
			uint32_t uint32[4];

		} clearColor;
	};

	struct RenderingInfo
	{
		StackVector<AttachmentInfo, MAX_COLOR_ATTACHMENT_COUNT> colorAttachments;
		AttachmentInfo depthAttachmentInfo{};

		Rect2D renderArea{};
		uint32_t layerCount = 1;
	};

	constexpr uint32_t ALL_MIPS = std::numeric_limits<uint32_t>::max();
	constexpr uint32_t ALL_LAYERS = std::numeric_limits<uint32_t>::max();

	struct ImageSubResource
	{
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = ALL_MIPS;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = ALL_LAYERS;
	};

	struct ImageBarrier
	{
		WeakPtr<RHIResource> resource;

		BarrierStage srcStage = BarrierStage::None;
		BarrierStage dstStage = BarrierStage::None;
	
		BarrierAccess srcAccess = BarrierAccess::None;
		BarrierAccess dstAccess = BarrierAccess::None;
	
		ImageLayout srcLayout = ImageLayout::Undefined;
		ImageLayout dstLayout = ImageLayout::Undefined;

		ImageSubResource subResource;
	};

	struct BufferBarrier
	{
		WeakPtr<RHIResource> resource;

		BarrierStage srcStage = BarrierStage::None;
		BarrierStage dstStage = BarrierStage::None;

		BarrierAccess srcAccess = BarrierAccess::None;
		BarrierAccess dstAccess = BarrierAccess::None;

		uint64_t size = 0;
		uint64_t offset = 0;
	};

	struct GlobalBarrier
	{
		BarrierStage srcStage = BarrierStage::None;
		BarrierStage dstStage = BarrierStage::None;

		BarrierAccess srcAccess = BarrierAccess::None;
		BarrierAccess dstAccess = BarrierAccess::None;
	};

	struct ResourceBarrierInfo
	{
		~ResourceBarrierInfo() = default;

		BarrierType type = BarrierType::Global;

		ImageBarrier& imageBarrier() { return m_barrier.Get<ImageBarrier>(); }
		BufferBarrier& bufferBarrier() { return m_barrier.Get<BufferBarrier>(); }
		GlobalBarrier& globalBarrier() { return m_barrier.Get<GlobalBarrier>(); }

		const ImageBarrier& imageBarrier() const { return m_barrier.Get<ImageBarrier>(); }
		const BufferBarrier& bufferBarrier() const { return m_barrier.Get<BufferBarrier>(); }
		const GlobalBarrier& globalBarrier() const { return m_barrier.Get<GlobalBarrier>(); }

	private:
		Variant<ImageBarrier, BufferBarrier, GlobalBarrier> m_barrier;
	};

	struct IndirectIndexedCommand
	{
		uint32_t indexCount;
		uint32_t instanceCount;
		uint32_t firstIndex;
		int32_t vertexOffset;
		uint32_t firstInstance;
	};

	struct IndirectDrawCommand
	{
		uint32_t vertexCount;
		uint32_t instanceCount;
		uint32_t firstVertex;
		uint32_t firstInstance;
	};

	struct IndirectDispatchCommand
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;
	};

	struct IndirectMeshTasksCommand
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;
	};
}
