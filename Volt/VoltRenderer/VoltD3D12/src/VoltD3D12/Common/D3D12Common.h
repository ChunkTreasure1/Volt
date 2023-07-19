#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>

#include <string>
#include <istream>

#include "VoltRHI/Graphics/GraphicsContext.h"

namespace Volt
{
	template<typename T>
	using WinRef = Microsoft::WRL::ComPtr<T>;

	class DxException
	{
	public:
		DxException() = default;
		DxException(HRESULT hr, const std::string& functionName, const std::string& filename, int lineNumber)
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


		HRESULT ErrorCode = S_OK;
		std::string FunctionName;
		std::string Filename;
		int32_t LineNumber = -1;
	};
}

#define VT_D3D12_ID(x) IID_PPV_ARGS(x.GetAddressOf())

#define VT_D3D12_DELETE(x) if(x) x->Release()

#define VT_D3D12_CHECK(x) \
{ \
HRESULT hr__ = (x); \
std::string str = __FILE__; \
if(FAILED(hr__)) {  DxException ex(hr__, #x, str,__LINE__); GraphicsContext::Log(Severity::Error, ex.ToString()); } } \
