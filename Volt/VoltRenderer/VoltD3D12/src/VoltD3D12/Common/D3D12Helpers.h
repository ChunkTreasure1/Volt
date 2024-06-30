#pragma once

#include <VoltRHI/Core/RHICommon.h>
#include <VoltRHI/Shader/BufferLayout.h>

#include <d3d12.h>

namespace Volt::RHI
{
	namespace Utility
	{
		inline static D3D12_FILL_MODE VoltToD3D12Fill(FillMode fillMode)
		{
			switch (fillMode)
			{
				case FillMode::Solid: return D3D12_FILL_MODE_SOLID;
				case FillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
			}

			assert(false && "Fill mode not supported!");
			return D3D12_FILL_MODE_SOLID;
		}

		inline static D3D12_CULL_MODE VoltToD3D12Cull(CullMode cullMode)
		{
			switch (cullMode)
			{
				case CullMode::Front: return D3D12_CULL_MODE_FRONT;
				case CullMode::Back: return D3D12_CULL_MODE_BACK;
				case CullMode::FrontAndBack: return D3D12_CULL_MODE_NONE;
				case CullMode::None: return D3D12_CULL_MODE_NONE;
			}

			assert(false && "Cull mode not supported!");
			return D3D12_CULL_MODE_BACK;
		}

		inline D3D12_COMPARISON_FUNC VoltToD3D12CompareOp(CompareOperator compareOp)
		{
			switch (compareOp)
			{
				case CompareOperator::None: return D3D12_COMPARISON_FUNC_NEVER;
				case CompareOperator::Never: return D3D12_COMPARISON_FUNC_NEVER;
				case CompareOperator::Less: return D3D12_COMPARISON_FUNC_LESS;
				case CompareOperator::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
				case CompareOperator::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
				case CompareOperator::Greater: return D3D12_COMPARISON_FUNC_GREATER;
				case CompareOperator::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
				case CompareOperator::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
			}

			assert(false && "Compare operator not supported!");
			return D3D12_COMPARISON_FUNC_LESS;
		}

		inline static D3D12_PRIMITIVE_TOPOLOGY_TYPE VoltToD3D12Topology(Topology topology)
		{
			switch (topology)
			{
				case Topology::TriangleList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				case Topology::LineList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
				case Topology::TriangleStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				case Topology::PatchList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
				case Topology::PointList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

			}

			assert(false && "Topology not supported!");
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}

		inline static DXGI_FORMAT VoltToD3D12ElementFormat(ElementType type)
		{
			switch (type)
			{
				case ElementType::Bool: return DXGI_FORMAT_R8_UINT;

				case ElementType::Byte: return DXGI_FORMAT_R8_UINT;
				case ElementType::Byte2: return DXGI_FORMAT_R8G8_UINT;
				case ElementType::Byte3: return DXGI_FORMAT_R8G8B8A8_UINT;
				case ElementType::Byte4: return DXGI_FORMAT_R8G8B8A8_UINT;

				case ElementType::Half: return DXGI_FORMAT_R16_FLOAT;
				case ElementType::Half2: return DXGI_FORMAT_R16G16_FLOAT;
				case ElementType::Half3: return DXGI_FORMAT_R16G16B16A16_FLOAT;
				case ElementType::Half4: return DXGI_FORMAT_R16G16B16A16_FLOAT;

				case ElementType::UShort: return DXGI_FORMAT_R16_UINT;
				case ElementType::UShort2: return DXGI_FORMAT_R16G16_UINT;
				case ElementType::UShort3: return DXGI_FORMAT_R16G16B16A16_UINT;
				case ElementType::UShort4: return DXGI_FORMAT_R16G16B16A16_UINT;

				case ElementType::Int: return DXGI_FORMAT_R32_SINT;
				case ElementType::Int2: return DXGI_FORMAT_R32G32_SINT;
				case ElementType::Int3: return DXGI_FORMAT_R32G32B32_SINT;
				case ElementType::Int4: return DXGI_FORMAT_R32G32B32A32_SINT;

				case ElementType::UInt: return DXGI_FORMAT_R32_UINT;
				case ElementType::UInt2: return DXGI_FORMAT_R32G32_UINT;
				case ElementType::UInt3: return DXGI_FORMAT_R32G32B32_UINT;
				case ElementType::UInt4: return DXGI_FORMAT_R32G32B32A32_UINT;

				case ElementType::Float: return DXGI_FORMAT_R32_FLOAT;
				case ElementType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
				case ElementType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
				case ElementType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;

				case ElementType::Float3x3: return DXGI_FORMAT_R32G32B32_FLOAT;
				case ElementType::Float4x4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		inline static bool DoFormatSupportUnorderedAccess(DXGI_FORMAT format)
		{
			switch (format)
			{
				case DXGI_FORMAT_R32G32B32A32_FLOAT:
				case DXGI_FORMAT_R32G32B32A32_UINT:
				case DXGI_FORMAT_R32G32B32A32_SINT:
				case DXGI_FORMAT_R16G16B16A16_FLOAT:
				case DXGI_FORMAT_R16G16B16A16_UINT:
				case DXGI_FORMAT_R16G16B16A16_SINT:
				case DXGI_FORMAT_R16G16B16A16_UNORM:
				case DXGI_FORMAT_R16G16B16A16_SNORM:
				case DXGI_FORMAT_R10G10B10A2_UNORM:
				case DXGI_FORMAT_R10G10B10A2_UINT:
				case DXGI_FORMAT_R8G8B8A8_UNORM:
				case DXGI_FORMAT_R8G8B8A8_UINT:
				case DXGI_FORMAT_R8G8B8A8_SINT:
				case DXGI_FORMAT_R11G11B10_FLOAT:
				case DXGI_FORMAT_R32G32_FLOAT:
				case DXGI_FORMAT_R32G32_UINT:
				case DXGI_FORMAT_R32G32_SINT:
				case DXGI_FORMAT_R16G16_FLOAT:
				case DXGI_FORMAT_R16G16_UINT:
				case DXGI_FORMAT_R16G16_SINT:
				case DXGI_FORMAT_R16G16_UNORM:
				case DXGI_FORMAT_R16G16_SNORM:
				case DXGI_FORMAT_R8G8_UINT:
				case DXGI_FORMAT_R8G8_SINT:
				case DXGI_FORMAT_R8G8_UNORM:
				case DXGI_FORMAT_R8G8_SNORM:
				case DXGI_FORMAT_R32_FLOAT:
				case DXGI_FORMAT_R32_UINT:
				case DXGI_FORMAT_R32_SINT:
				case DXGI_FORMAT_R16_FLOAT:
				case DXGI_FORMAT_R16_UINT:
				case DXGI_FORMAT_R16_UNORM:
				case DXGI_FORMAT_R16_SNORM:
				case DXGI_FORMAT_R16_SINT:
				case DXGI_FORMAT_R8_UNORM:
				case DXGI_FORMAT_R8_UINT:
				case DXGI_FORMAT_R8_SINT:
				case DXGI_FORMAT_R8_SNORM:
					return true;
			}

			return false;
		}
	}
}
