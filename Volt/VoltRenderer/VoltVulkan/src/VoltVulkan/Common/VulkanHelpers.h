#pragma once

#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Images/ImageUtility.h"
#include "VoltRHI/Shader/BufferLayout.h"

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	namespace Utility
	{
		inline static constexpr VkFormat VoltToVulkanFormat(Format format)
		{
			return static_cast<VkFormat>(format);
		}

		inline static constexpr Format VulkanToVoltFormat(VkFormat format)
		{
			return static_cast<Format>(format);
		}

		inline static constexpr VkPresentModeKHR VoltToVulkanPresentMode(PresentMode presentMode)
		{
			return static_cast<VkPresentModeKHR>(presentMode);
		}

		inline static constexpr VkColorSpaceKHR VoltToVulkanColorSpace(ColorSpace colorSpace)
		{
			return static_cast<VkColorSpaceKHR>(colorSpace);
		}
		
		inline static constexpr VkShaderStageFlagBits VoltToVulkanShaderStage(ShaderStage stage)
		{
			return static_cast<VkShaderStageFlagBits>(stage);
		}

		inline static VkPrimitiveTopology VoltToVulkanTopology(Topology topology)
		{
			switch (topology)
			{
				case Topology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				case Topology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				case Topology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
				case Topology::PatchList: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
				case Topology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

			}

			assert(false && "Topology not supported!");
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		inline static VkPolygonMode VoltToVulkanFill(FillMode fillMode)
		{
			switch (fillMode)
			{
				case FillMode::Solid: return VK_POLYGON_MODE_FILL;
				case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;
			}

			assert(false && "Fill mode not supported!");
			return VK_POLYGON_MODE_FILL;
		}

		inline static VkCullModeFlags VoltToVulkanCull(CullMode cullMode)
		{
			switch (cullMode)
			{
				case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
				case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
				case CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
				case CullMode::None: return VK_CULL_MODE_NONE;
			}

			assert(false && "Cull mode not supported!");
			return VK_CULL_MODE_BACK_BIT;
		}

		inline static VkFormat VoltToVulkanElementFormat(ElementType type)
		{
			switch (type)
			{
				case ElementType::Bool: return VK_FORMAT_R8_UINT;

				case ElementType::Byte: return VK_FORMAT_R8_UINT;
				case ElementType::Byte2: return VK_FORMAT_R8G8_UINT;
				case ElementType::Byte3: return VK_FORMAT_R8G8B8_UINT;
				case ElementType::Byte4: return VK_FORMAT_R8G8B8A8_UINT;

				case ElementType::Half: return VK_FORMAT_R16_SFLOAT;
				case ElementType::Half2: return VK_FORMAT_R16G16_SFLOAT;
				case ElementType::Half3: return VK_FORMAT_R16G16B16_SFLOAT;
				case ElementType::Half4: return VK_FORMAT_R16G16B16A16_SFLOAT;

				case ElementType::UShort: return VK_FORMAT_R16_UINT;
				case ElementType::UShort2: return VK_FORMAT_R16G16_UINT;
				case ElementType::UShort3: return VK_FORMAT_R16G16B16_UINT;
				case ElementType::UShort4: return VK_FORMAT_R16G16B16A16_UINT;

				case ElementType::Int: return VK_FORMAT_R32_SINT;
				case ElementType::Int2: return VK_FORMAT_R32G32_SINT;
				case ElementType::Int3: return VK_FORMAT_R32G32B32_SINT;
				case ElementType::Int4: return VK_FORMAT_R32G32B32A32_SINT;

				case ElementType::UInt: return VK_FORMAT_R32_UINT;
				case ElementType::UInt2: return VK_FORMAT_R32G32_UINT;
				case ElementType::UInt3: return VK_FORMAT_R32G32B32_UINT;
				case ElementType::UInt4: return VK_FORMAT_R32G32B32A32_UINT;

				case ElementType::Float: return VK_FORMAT_R32_SFLOAT;
				case ElementType::Float2: return VK_FORMAT_R32G32_SFLOAT;
				case ElementType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
				case ElementType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;

				case ElementType::Float3x3: return VK_FORMAT_R32G32B32_SFLOAT;
				case ElementType::Float4x4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}

			return VK_FORMAT_R8G8B8A8_UNORM;
		}

		inline VkCompareOp VoltToVulkanCompareOp(CompareOperator compareOp)
		{
			switch (compareOp)
			{
				case CompareOperator::None: return VK_COMPARE_OP_NEVER;
				case CompareOperator::Never: return VK_COMPARE_OP_NEVER;
				case CompareOperator::Less: return VK_COMPARE_OP_LESS;
				case CompareOperator::Equal: return VK_COMPARE_OP_EQUAL;
				case CompareOperator::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
				case CompareOperator::Greater: return VK_COMPARE_OP_GREATER;
				case CompareOperator::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
				case CompareOperator::Always: return VK_COMPARE_OP_ALWAYS;
			}

			assert(false && "Compare operator not supported!");
			return VK_COMPARE_OP_LESS;
		}

		inline VkImageViewType VoltToVulkanViewType(ImageViewType viewType)
		{
			switch (viewType)
			{
				case ImageViewType::View1D: return VK_IMAGE_VIEW_TYPE_1D;
				case ImageViewType::View2D: return VK_IMAGE_VIEW_TYPE_2D;
				case ImageViewType::View3D: return VK_IMAGE_VIEW_TYPE_3D;
				case ImageViewType::View1DArray: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
				case ImageViewType::View2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				case ImageViewType::ViewCube: return VK_IMAGE_VIEW_TYPE_CUBE;
				case ImageViewType::ViewCubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			}

			return VK_IMAGE_VIEW_TYPE_2D;
		}

		inline VkAttachmentLoadOp VoltToVulkanLoadOp(ClearMode clearMode)
		{
			switch (clearMode)
			{
				case ClearMode::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
				case ClearMode::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
				case ClearMode::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}

			return VK_ATTACHMENT_LOAD_OP_LOAD;
		}

		inline VkFilter VoltToVulkanFilter(TextureFilter filter)
		{
			switch (filter)
			{
				case TextureFilter::None: break;
				case TextureFilter::Linear: return VK_FILTER_LINEAR;
				case TextureFilter::Nearest: return VK_FILTER_NEAREST;
			}

			return VK_FILTER_LINEAR;
		}

		inline VkSamplerAddressMode VoltToVulkanWrapMode(TextureWrap wrap)
		{
			switch (wrap)
			{
				case TextureWrap::None: break;
				case TextureWrap::Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				case TextureWrap::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}

			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}

		inline VkSamplerMipmapMode VoltToVulkanMipMapMode(TextureFilter filter)
		{
			switch (filter)
			{
				case TextureFilter::None: break;
				case TextureFilter::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
				case TextureFilter::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			}

			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}

		inline static VkBufferUsageFlags GetVkBufferUsageFlags(BufferUsage usageFlags)
		{
			VkBufferUsageFlags result = 0;

			if ((usageFlags & BufferUsage::TransferSrc) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			}

			if ((usageFlags & BufferUsage::TransferDst) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			}

			if ((usageFlags & BufferUsage::UniformBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			}

			if ((usageFlags & BufferUsage::StorageBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			}

			if ((usageFlags & BufferUsage::IndexBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			}

			if ((usageFlags & BufferUsage::VertexBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			}

			if ((usageFlags & BufferUsage::IndirectBuffer) != BufferUsage::None)
			{
				result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			}

			return result;
		}

		inline static VkImageUsageFlags GetVkImageUsageFlags(ImageUsage usageFlags, Format imageFormat)
		{
			VkImageUsageFlags result = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

			if (usageFlags == ImageUsage::Attachment)
			{
				result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

				if (Utility::IsDepthFormat(imageFormat))
				{
					result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				}
				else
				{
					result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				}
			}
			else if (usageFlags == ImageUsage::AttachmentStorage)
			{
				result |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

				if (Utility::IsDepthFormat(imageFormat))
				{
					result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				}
				else
				{
					result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				}
			}
			else if (usageFlags == ImageUsage::Texture)
			{
				result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}
			else if (usageFlags == ImageUsage::Storage)
			{
				result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			}

			return result;
		}
	}
}
