#pragma once

#include <wrl.h>
#include <d3d12.h>
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
		DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
		{
			ErrorCode = hr;
			FunctionName = functionName;
			Filename = filename;
			LineNumber = lineNumber;
		}
		std::wstring ToWString()const
		{
			auto errorStr = std::system_category().message(ErrorCode);
			std::wstring errorWstr(errorStr.begin(), errorStr.end());
			return L"Error: " + errorWstr + L" in Function: " + FunctionName + L" In file: " + Filename + L" at line: " + std::to_wstring(LineNumber);
		}

		std::string ToString()
		{
			const std::wstring wstr = ToWString();
			
			return {wstr.cbegin(), wstr.cend()};
		}

		HRESULT ErrorCode = S_OK;
		std::wstring FunctionName;
		std::wstring Filename;
		int32_t LineNumber = -1;
	};
}

#define VT_D3D12_ID(x) IID_PPV_ARGS(x.GetAddressOf())
#define VT_D3D12_CHECK(x) \
{ \
HRESULT hr__ = (x); \
std::string str = __FILE__; \
std::wstring wfn(str.begin(), str.end()); \
if(FAILED(hr__)) {  ::Volt::DxException ex(hr__, L#x, wfn,__LINE__); ::Volt::GraphicsContext::Log("{}", ex.ToString()); } } \
