#pragma once

#include <map>
#include <unordered_map>
#include <set>

#include <string>

#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>

#include <functional>
#include <algorithm>
#include <filesystem>

#include <future>

//Windows stuff
#include <wrl.h>
#include <d3d12/d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3d12/d3d12sdklayers.h>
#include <d3d12/d3dx12/d3dx12.h>

#include <string>
#include <istream>

#include "RHIModule/Graphics/GraphicsContext.h"
#include "RHIModule/Core/RHICommon.h"

#include <LogModule/Log.h>

VT_DECLARE_LOG_CATEGORY(LogD3D12RHI, LogVerbosity::Trace);

namespace Volt
{

	class DxException
	{
	public:
		DxException() = default;
		DxException(long hr, const std::string& functionName, const std::string& filename, int lineNumber)
		{
			ErrorCode = hr;
			FunctionName = functionName;
			Filename = filename;
			LineNumber = lineNumber;
		}
		std::string ToString()const
		{
			auto errorStr = std::system_category().message(ErrorCode);
			return "Error: " + errorStr + " in Function: " + FunctionName + " In file: " + Filename + " at line: " + std::to_string(LineNumber);
		}


		long ErrorCode = 0;
		std::string FunctionName;
		std::string Filename;
		int32_t LineNumber = -1;
	};

	namespace RHI
	{
		inline D3D12_COMMAND_LIST_TYPE GetD3D12QueueType(QueueType type)
		{
			D3D12_COMMAND_LIST_TYPE d3d12Type = {};
			switch (type)
			{
				case QueueType::Graphics:
					d3d12Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
					break;
				case QueueType::Compute:
					d3d12Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
					break;
				case QueueType::TransferCopy:
					d3d12Type = D3D12_COMMAND_LIST_TYPE_COPY;
					break;
			}
			return d3d12Type;
		}

		inline DXGI_FORMAT ConvertFormatToD3D12Format(PixelFormat format)
		{
			switch (format)
			{
				case PixelFormat::UNDEFINED: return DXGI_FORMAT_UNKNOWN;
				case PixelFormat::R4G4_UNORM_PACK8: return DXGI_FORMAT_UNKNOWN;
				case PixelFormat::R4G4B4A4_UNORM_PACK16: return DXGI_FORMAT_UNKNOWN;
				case PixelFormat::B4G4R4A4_UNORM_PACK16: return DXGI_FORMAT_B4G4R4A4_UNORM;
				case PixelFormat::R5G6B5_UNORM_PACK16: return DXGI_FORMAT_B5G6R5_UNORM;
				case PixelFormat::B5G6R5_UNORM_PACK16: return DXGI_FORMAT_B5G6R5_UNORM;
				case PixelFormat::R5G5B5A1_UNORM_PACK16: return DXGI_FORMAT_B5G5R5A1_UNORM;
				case PixelFormat::B5G5R5A1_UNORM_PACK16: return DXGI_FORMAT_B5G5R5A1_UNORM;
				case PixelFormat::A1R5G5B5_UNORM_PACK16: return DXGI_FORMAT_B5G5R5A1_UNORM;
				case PixelFormat::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
				case PixelFormat::R8_SNORM: return DXGI_FORMAT_R8_SNORM;
				case PixelFormat::R8_UINT: return DXGI_FORMAT_R8_UINT;
				case PixelFormat::R8_SINT: return DXGI_FORMAT_R8_SINT;
				case PixelFormat::R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
				case PixelFormat::R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
				case PixelFormat::R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
				case PixelFormat::R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
				case PixelFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
				case PixelFormat::R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
				case PixelFormat::R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
				case PixelFormat::R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
				case PixelFormat::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
				case PixelFormat::R16_UNORM: return DXGI_FORMAT_R16_UNORM;
				case PixelFormat::R16_SNORM: return DXGI_FORMAT_R16_SNORM;
				case PixelFormat::R16_UINT: return DXGI_FORMAT_R16_UINT;
				case PixelFormat::R16_SINT: return DXGI_FORMAT_R16_SINT;
				case PixelFormat::R16_SFLOAT: return DXGI_FORMAT_R16_FLOAT;
				case PixelFormat::R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
				case PixelFormat::R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
				case PixelFormat::R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
				case PixelFormat::R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
				case PixelFormat::R16G16_SFLOAT: return DXGI_FORMAT_R16G16_FLOAT;
				case PixelFormat::R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
				case PixelFormat::R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
				case PixelFormat::R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
				case PixelFormat::R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
				case PixelFormat::R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
				case PixelFormat::R32_UINT: return DXGI_FORMAT_R32_UINT;
				case PixelFormat::R32_SINT: return DXGI_FORMAT_R32_SINT;
				case PixelFormat::R32_SFLOAT: return DXGI_FORMAT_R32_FLOAT;
				case PixelFormat::R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
				case PixelFormat::R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
				case PixelFormat::R32G32_SFLOAT: return DXGI_FORMAT_R32G32_FLOAT;
				case PixelFormat::R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
				case PixelFormat::R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
				case PixelFormat::R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
				case PixelFormat::R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
				case PixelFormat::R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;

				case PixelFormat::D16_UNORM: return DXGI_FORMAT_R16_TYPELESS;
				case PixelFormat::X8_D24_UNORM_PACK32: return DXGI_FORMAT_R24G8_TYPELESS;
				case PixelFormat::D32_SFLOAT_S8_UINT: return DXGI_FORMAT_R32G8X24_TYPELESS;
				case PixelFormat::D32_SFLOAT: return DXGI_FORMAT_R32_TYPELESS;

				case PixelFormat::BC1_RGB_UNORM_BLOCK: return DXGI_FORMAT_BC1_UNORM;
				case PixelFormat::BC1_RGB_SRGB_BLOCK: return DXGI_FORMAT_BC1_UNORM_SRGB;
				case PixelFormat::BC1_RGBA_UNORM_BLOCK: return DXGI_FORMAT_BC1_UNORM;
				case PixelFormat::BC1_RGBA_SRGB_BLOCK: return DXGI_FORMAT_BC1_UNORM_SRGB;
				case PixelFormat::BC2_UNORM_BLOCK: return DXGI_FORMAT_BC2_UNORM;
				case PixelFormat::BC2_SRGB_BLOCK: return DXGI_FORMAT_BC2_UNORM_SRGB;
				case PixelFormat::BC3_UNORM_BLOCK: return DXGI_FORMAT_BC3_UNORM;
				case PixelFormat::BC3_SRGB_BLOCK: return DXGI_FORMAT_BC3_UNORM_SRGB;
				case PixelFormat::BC4_UNORM_BLOCK: return DXGI_FORMAT_BC4_UNORM;
				case PixelFormat::BC4_SNORM_BLOCK: return DXGI_FORMAT_BC4_SNORM;
				case PixelFormat::BC5_UNORM_BLOCK: return DXGI_FORMAT_BC5_UNORM;
				case PixelFormat::BC5_SNORM_BLOCK: return DXGI_FORMAT_BC5_SNORM;
				case PixelFormat::BC6H_UFLOAT_BLOCK: return DXGI_FORMAT_BC6H_UF16;
				case PixelFormat::BC6H_SFLOAT_BLOCK: return DXGI_FORMAT_BC6H_SF16;
				case PixelFormat::BC7_UNORM_BLOCK: return DXGI_FORMAT_BC7_UNORM;
				case PixelFormat::BC7_SRGB_BLOCK: return DXGI_FORMAT_BC7_UNORM_SRGB;

				case PixelFormat::B10G11R11_UFLOAT_PACK32: return DXGI_FORMAT_R11G11B10_FLOAT;
				case PixelFormat::A2B10G10R10_UNORM_PACK32: return DXGI_FORMAT_R10G10B10A2_UNORM;
			}

			VT_ENSURE(false);
			return DXGI_FORMAT_UNKNOWN;
		}
	}
}



#define VT_D3D12_ID(x) IID_PPV_ARGS(&x)
#define VT_D3D12_WRID(x) IID_PPV_ARGS(x.GetAddressOf())

#define VT_D3D12_DELETE(x) if(x){ x->Release(); x = nullptr; } 

#define VT_D3D12_CHECK(x) \
{ \
HRESULT hr__ = (x); \
std::string str = __FILE__; \
if(FAILED(hr__)) {  DxException ex((long)hr__, #x, str,__LINE__); VT_LOGC(Error, LogD3D12RHI, ex.ToString()); } } \

