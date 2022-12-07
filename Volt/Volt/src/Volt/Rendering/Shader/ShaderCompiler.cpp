#include "vtpch.h"
#include "ShaderCompiler.h"

#include "Volt/Log/Log.h"
#include "Volt/Rendering/Shader/ShaderUtility.h"

#include <d3dcompiler.h>

namespace Volt
{
	bool ShaderCompiler::TryCompile(const std::vector<std::filesystem::path>& aPaths, std::unordered_map<ShaderStage, ComPtr<ID3D10Blob>>& outCode)
	{
		const auto cacheDirectory = Utility::GetShaderCacheDirectory();

		for (const auto& path : aPaths)
		{
			if (!std::filesystem::exists(path))
			{
				VT_CORE_ERROR("Unable to compile shader {0}! It does not exist!", path.string().c_str());
				return false;
			}

			ShaderStage stage = Utility::GetStageFromPath(path);

			// Compile shader
			{
				uint32_t compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;

#ifdef VT_ENABLE_SHADER_DEBUG
				compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

				ID3D10Blob* error = nullptr;
				D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", Utility::GetProfileFromStage(stage).c_str(), compileFlags, 0, outCode[stage].GetAddressOf(), &error);
				if (error)
				{
					VT_CORE_ERROR("Shader {0} failed to compile with error: {1}", path.string().c_str(), (const char*)error->GetBufferPointer());
					error->Release();

					return false;
				}
			}

			// Cache shader
			{
				Utility::CreateCacheDirectoryIfNeeded();

				const auto extension = Utility::GetShaderStageCachedFileExtension(stage);
				const auto cachedPath = cacheDirectory / (path.filename().string() + extension);
				std::ofstream output(cachedPath, std::ios::binary | std::ios::out);
				if (output.is_open())
				{
					output.write((const char*)outCode[stage]->GetBufferPointer(), outCode[stage]->GetBufferSize());
					output.close();
				}
				else
				{
					VT_CORE_ERROR("Unable to write shader cache file {0}!", cachedPath.string());
				}
			}
		}

		return true;
	}
}

