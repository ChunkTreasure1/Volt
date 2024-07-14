#include "dxpch.h"
#include "D3D12Helpers.h"

namespace Volt::RHI::Utility
{
	D3D12_RESOURCE_DESC1 GetD3D12ResourceDesc(const ImageSpecification& specification)
	{
		D3D12_RESOURCE_DESC1 result{};

		if (specification.imageType == ResourceType::Image2D)
		{
			result.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		}
		else if (specification.imageType == ResourceType::Image3D)
		{
			result.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		}

		result.Alignment = 0;
		result.Width = specification.width;
		result.Height = specification.height;
		result.DepthOrArraySize = specification.imageType == ResourceType::Image2D ? static_cast<uint16_t>(specification.layers) : static_cast<uint16_t>(specification.depth);
		result.MipLevels = static_cast<uint16_t>(specification.mips);
		result.Format = ConvertFormatToD3D12Format(specification.format);
		result.SampleDesc = { 1, 0 };

		result.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (specification.usage == ImageUsage::Attachment || specification.usage == ImageUsage::AttachmentStorage)
		{
			if (Utility::IsDepthFormat(specification.format))
			{
				result.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}
			else
			{
				result.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}
		}

		if ((specification.memoryUsage & MemoryUsage::GPUToCPU) != MemoryUsage::None)
		{
			result.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		}
		else
		{
			result.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		}

		const bool resourceShouldBeUnorderedAccess = (specification.memoryUsage & MemoryUsage::GPU) != MemoryUsage::None && (specification.usage == ImageUsage::Storage || specification.usage == ImageUsage::AttachmentStorage);

		if (resourceShouldBeUnorderedAccess && Utility::DoFormatSupportUnorderedAccess(result.Format))
		{
			result.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		if (resourceShouldBeUnorderedAccess && !Utility::DoFormatSupportUnorderedAccess(result.Format))
		{
			RHILog::LogTagged(LogSeverity::Error, "[D3D12Allocator]", "Resource description is not valid for unordered access!");
		}

		return result;
	}

	D3D12_RESOURCE_ALLOCATION_INFO GetResourceAllocationInfo(const D3D12_RESOURCE_DESC1& resourceDesc)
	{
		D3D12_RESOURCE_ALLOCATION_INFO1 info1Unused;
		return GraphicsContext::GetDevice()->GetHandle<ID3D12Device8*>()->GetResourceAllocationInfo2(0, 1, &resourceDesc, &info1Unused);
	}

	MemoryRequirement GetMemoryRequirements(const D3D12_RESOURCE_DESC1& resourceDesc)
	{
		MemoryRequirement requirement{};

		if (resourceDesc.Alignment == 0 && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			requirement.memoryTypeBits = 0;
			requirement.alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			requirement.size = resourceDesc.Width;
		
			return requirement;
		}
	
		// #TODO_Ivar: implement a function that makes sure that this is possible
		//if (resourceDesc.Alignment == 0 && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
		//	(resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
		//{
		//	const uint64_t smallAlignmentToTry = resourceDesc.SampleDesc.Count > 1 ? D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT : D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
		//	
		//	auto tempDesc = resourceDesc;
		//	tempDesc.Alignment = smallAlignmentToTry;
		//
		//	auto allocInfo = GetResourceAllocationInfo(tempDesc);
		//	if (allocInfo.Alignment != smallAlignmentToTry)
		//	{
		//		allocInfo = GetResourceAllocationInfo(resourceDesc);
		//	}
		//
		//	requirement.alignment = allocInfo.Alignment;
		//	requirement.size = allocInfo.SizeInBytes;
		//}

		auto allocInfo = GetResourceAllocationInfo(resourceDesc);
		requirement.alignment = allocInfo.Alignment;
		requirement.size = allocInfo.SizeInBytes;
		return requirement;
	}
}
