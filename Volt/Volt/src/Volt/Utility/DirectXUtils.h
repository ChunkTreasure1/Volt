#pragma once

#define _HAS_STD_BYTE 0
#include <Windows.h>

#include <stringapiset.h>
#include <string>

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT aHResult, const std::wstring& aFunctionName, const std::wstring& aFilename, int aLineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};


inline std::wstring AnsiToWString(const std::string& aString)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, aString.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#ifdef VT_DEBUG
#define VT_DX_CHECK(x)													  \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#else
#define VT_DX_CHECK(x) x

#endif