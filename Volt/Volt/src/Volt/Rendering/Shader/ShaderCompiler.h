#pragma once

#include "Volt/Rendering/Shader/ShaderCommon.h"

#include <wrl.h>

struct ID3D11DeviceChild;
struct ID3D10Blob;

using namespace Microsoft::WRL;

namespace Volt
{
	class ShaderCompiler
	{
	public:
		static bool TryCompile(const std::vector<std::filesystem::path>& aPaths, std::unordered_map<ShaderStage, ComPtr<ID3D10Blob>>& outCode);

	private:
		ShaderCompiler() = delete;
	};
}