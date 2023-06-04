#include "vtpch.h"
#include "HLSLIncluder.h"

namespace Volt
{
	HRESULT HLSLIncluder::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
	{
		static IDxcUtils* utils = nullptr;
		if (!utils)
		{
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
			if (FAILED(utils->CreateDefaultIncludeHandler(&s_defaultIncludeHandler)))
			{
				VT_CORE_ASSERT(false, "Failed to create default HLSL include handler!");
			}
		}

		const std::filesystem::path filepath = pFilename;
		if (myIncludedFiles.contains(filepath))
		{
			static const char nullStr[] = " ";

			IDxcBlobEncoding* encoding;
			utils->CreateBlob(nullStr, ARRAYSIZE(nullStr), CP_UTF8, &encoding);
			*ppIncludeSource = encoding;

			return S_OK;
		}

		if (!std::filesystem::exists(filepath))
		{
			return S_FALSE;
		}

		std::ifstream stream{};
		stream.open(filepath);

		if (!stream.is_open())
		{
			VT_CORE_ERROR("Failed to read file {0}", filepath.string().c_str());
			return S_FALSE;
		}

		myIncludedFiles.insert(filepath);

		std::stringstream buffer;
		buffer << stream.rdbuf();
		stream.close();

		std::string data = buffer.str();

		IDxcBlobEncoding* encoding;
		utils->CreateBlob(data.data(), (uint32_t)data.size(), CP_UTF8, &encoding);

		*ppIncludeSource = encoding;
		return S_OK;
	}
}