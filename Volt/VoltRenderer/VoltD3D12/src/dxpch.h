#pragma once

#include <vector>
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
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3d12sdklayers.h>
#include "VoltD3D12/Common/d3dx12.h"

#include <string>
#include <istream>

#include "VoltRHI/RHILog.h"
#include "VoltRHI/Graphics/GraphicsContext.h"
#include <VoltRHI/Core/RHICommon.h>

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
				case PixelFormat::R8_USCALED: return DXGI_FORMAT_R8_UINT;
				case PixelFormat::R8_SSCALED: return DXGI_FORMAT_R8_SINT;
				case PixelFormat::R8_UINT: return DXGI_FORMAT_R8_UINT;
				case PixelFormat::R8_SINT: return DXGI_FORMAT_R8_SINT;
				case PixelFormat::R8_SRGB: return DXGI_FORMAT_R8_UNORM; // No equivalent
				case PixelFormat::R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
				case PixelFormat::R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
				case PixelFormat::R8G8_USCALED: return DXGI_FORMAT_R8G8_UINT;
				case PixelFormat::R8G8_SSCALED: return DXGI_FORMAT_R8G8_SINT;
				case PixelFormat::R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
				case PixelFormat::R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
				case PixelFormat::R8G8_SRGB: return DXGI_FORMAT_R8G8_UNORM; // No equivalent
				case PixelFormat::R8G8B8_UNORM: return DXGI_FORMAT_UNKNOWN; // No equivalent
				case PixelFormat::R8G8B8_SNORM: return DXGI_FORMAT_UNKNOWN; // No equivalent
				case PixelFormat::R8G8B8_USCALED: return DXGI_FORMAT_UNKNOWN; // No equivalent
				case PixelFormat::R8G8B8_SSCALED: return DXGI_FORMAT_UNKNOWN; // No equivalent
				case PixelFormat::R8G8B8_UINT: return DXGI_FORMAT_UNKNOWN; // No equivalent
				case PixelFormat::R8G8B8_SINT: return DXGI_FORMAT_UNKNOWN; // No equivalent
				case PixelFormat::R8G8B8_SRGB: return DXGI_FORMAT_UNKNOWN; // No equivalent
				case PixelFormat::B8G8R8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
				case PixelFormat::B8G8R8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
				case PixelFormat::B8G8R8_USCALED: return DXGI_FORMAT_R8G8B8A8_UINT;
				case PixelFormat::B8G8R8_SSCALED: return DXGI_FORMAT_R8G8B8A8_SINT;
				case PixelFormat::B8G8R8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
				case PixelFormat::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
				case PixelFormat::X8_D24_UNORM_PACK32: return DXGI_FORMAT_D24_UNORM_S8_UINT;
				case PixelFormat::D16_UNORM_S8_UINT: return DXGI_FORMAT_UNKNOWN; // No equivalent
				case PixelFormat::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
				case PixelFormat::D32_SFLOAT_S8_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
				case PixelFormat::D32_SFLOAT: return DXGI_FORMAT_D32_FLOAT;
				case PixelFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
			}
			return {};
		}
	}
}



#define VT_D3D12_ID(x) IID_PPV_ARGS(&x)
#define VT_D3D12_WRID(x) IID_PPV_ARGS(x.GetAddressOf())

#define VT_D3D12_DELETE(x) if(x){ x->Release(); } 

#define VT_D3D12_CHECK(x) \
{ \
HRESULT hr__ = (x); \
std::string str = __FILE__; \
if(FAILED(hr__)) {  DxException ex((long)hr__, #x, str,__LINE__); RHILog::Log(LogSeverity::Error, ex.ToString()); } } \

