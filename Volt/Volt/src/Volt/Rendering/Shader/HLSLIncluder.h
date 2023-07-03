#pragma once

#include <Windows.h>

#include <unordered_set>
#include <filesystem>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

#include <dxc/dxcapi.h>

namespace Volt
{
	class HLSLIncluder : public IDxcIncludeHandler
	{
	public:
		HRESULT LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override;
		HRESULT QueryInterface(const IID& riid, void** ppvObject) override { return s_defaultIncludeHandler->QueryInterface(riid, ppvObject); }

		ULONG AddRef() override { return 0; }
		ULONG Release() override { return 0; }

	private:
		inline static IDxcIncludeHandler* s_defaultIncludeHandler = nullptr;
		std::unordered_set<std::filesystem::path> myIncludedFiles;
	};
}
