#include "vkpch.h"
#include "HLSLIncluder.h"

#include <VoltRHI/Graphics/GraphicsContext.h>

namespace Volt::RHI
{
	HLSLIncluder::HLSLIncluder()
	{
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_hlslUtils));
		HRESULT result = m_hlslUtils->CreateDefaultIncludeHandler(&m_defaultIncludeHandler);
		
		if (result != S_OK)
		{
			assert(false && "Failed to create default HLSL include handler");
		}
	}

	HLSLIncluder::~HLSLIncluder()
	{
		m_defaultIncludeHandler->Release();
		m_hlslUtils->Release();
	}

	HRESULT HLSLIncluder::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
	{
		const std::filesystem::path filepath = pFilename;
		if (m_includedFiles.contains(filepath))
		{
			static const char nullStr[] = " ";

			IDxcBlobEncoding* encoding = nullptr;
			m_hlslUtils->CreateBlob(nullStr, ARRAYSIZE(nullStr), CP_UTF8, &encoding);

			*ppIncludeSource = encoding;

			return S_OK;
		}

		if (!std::filesystem::exists(filepath))
		{
			return S_FALSE;
		}

		std::ifstream inStream{};
		inStream.open(filepath);

		if (!inStream)
		{
			GraphicsContext::Log(Severity::Error, std::format("[HLSLIncluder] Failed to read file {0}!", filepath.string()));
			return S_FALSE;
		}

		m_includedFiles.insert(filepath);

		std::stringstream buffer;
		buffer << inStream.rdbuf();
		inStream.close();

		std::string data = buffer.str();

		IDxcBlobEncoding* encoding = nullptr;
		m_hlslUtils->CreateBlob(data.data(), static_cast<uint32_t>(data.size()), CP_UTF8, &encoding);

		*ppIncludeSource = encoding;
		return S_OK;
	}
}