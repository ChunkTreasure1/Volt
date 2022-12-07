#include "vtpch.h"
#include "DirectXUtils.h"

#include <comdef.h>

DxException::DxException(HRESULT aHResult, const std::wstring& aFunctionName, const std::wstring& aFilename, int aLineNumber)
	: ErrorCode(aHResult),
	FunctionName(aFunctionName),
	Filename(aFilename),
	LineNumber(aLineNumber)
{}

std::wstring DxException::ToString()const
{
	// Get the string description of the error code.
	_com_error err(ErrorCode);
	std::wstring msg = err.ErrorMessage();

	return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}