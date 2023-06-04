#include "vtpch.h"
#include "ShaderCompiler.h"

#include "Volt/Rendering/Shader/HLSLIncluder.h"
#include "Volt/Rendering/Shader/ShaderUtility.h"
#include "Volt/Project/ProjectManager.h"

#include "Volt/Asset/AssetManager.h"

#include <shaderc/shaderc.hpp>
#include <dxc/dxcapi.h>

#include <file_includer.h>
#include <libshaderc_util/file_finder.h>

namespace Volt
{
	namespace Utility
	{
		inline const Shader::Language LanuguageFromPath(const std::filesystem::path& path)
		{
			if (path.extension().string() == ".glsl" || path.extension().string() == ".glsli" || path.extension().string() == ".glslh")
			{
				return Shader::Language::GLSL;
			}
			else if (path.extension().string() == ".hlsl" || path.extension().string() == ".hlsli" || path.extension().string() == ".hlslh")
			{
				return Shader::Language::HLSL;
			}

			return Shader::Language::Invalid;
		}

		inline const std::string GetErrorStringFromResult(IDxcResult* result)
		{
			std::string output;

			IDxcBlobUtf8* errors;
			result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
			if (errors && errors->GetStringLength() > 0)
			{
				output = (char*)errors->GetBufferPointer();
				errors->Release();
			}

			return output;
		}
	}

	bool ShaderCompiler::TryCompile(ShaderDataMap& outShaderData, const std::vector<std::filesystem::path>& shaderFiles)
	{
		ShaderSourceMap shaderSources;
		LoadShaderFromFiles(shaderSources, shaderFiles);

		return CompileAll(shaderSources, shaderFiles, outShaderData);
	}

	void ShaderCompiler::LoadShaderFromFiles(ShaderSourceMap& shaderSources, const std::vector<std::filesystem::path>& shaderFiles)
	{
		const auto shaderLanguages = GetLanguages(shaderFiles);

		for (uint32_t i = 0; const auto & path : shaderFiles)
		{
			VkShaderStageFlagBits stage = Utility::GetShaderStageFromFilename(path.filename().string());
			std::string source = Utility::ReadStringFromFile(AssetManager::GetContextPath(path) / path);

			if (shaderSources.find(stage) != shaderSources.end())
			{
				VT_CORE_ERROR("Multiple shaders of same stage defined in file {0}!", path.string().c_str());
				return;
			}

			shaderSources[stage].source = source;
			shaderSources[stage].filepath = path;
			shaderSources[stage].language = shaderLanguages.at(i);

			i++;
		}
	}

	const std::vector<Shader::Language> ShaderCompiler::GetLanguages(const std::vector<std::filesystem::path>& paths)
	{
		std::vector<Shader::Language> languages;

		for (const auto& path : paths)
		{
			languages.emplace_back(Utility::LanuguageFromPath(path));
		}

		return languages;
	}

	bool ShaderCompiler::CompileAll(const ShaderSourceMap& shaderSources, const std::vector<std::filesystem::path>& shaderFiles, ShaderDataMap& outShaderData)
	{
		for (const auto& [stage, stageData] : shaderSources)
		{
			if (stageData.language == Shader::Language::GLSL)
			{
				if (!CompileGLSL(stage, stageData.source, stageData.filepath, outShaderData[stage]))
				{
					return false;
				}
			}
			else if (stageData.language == Shader::Language::HLSL)
			{
				if (!CompileHLSL(stage, stageData.source, stageData.filepath, outShaderData[stage]))
				{
					return false;
				}
			}
		}

		return true;
	}

	bool ShaderCompiler::CompileGLSL(const VkShaderStageFlagBits stage, const std::string& source, const std::filesystem::path filepath, std::vector<uint32_t>& outShaderData)
	{
		const auto cacheDirectory = Utility::GetShaderCacheDirectory();
		const auto extension = Utility::GetShaderStageCachedFileExtension(stage);

		const auto cachedPath = cacheDirectory / (filepath.filename().string() + extension);

		shaderc::Compiler compiler;
		shaderc::CompileOptions compileOptions;

		shaderc_util::FileFinder fileFinder;
		fileFinder.search_path().emplace_back("Engine/Shaders/Source/Includes");
		fileFinder.search_path().emplace_back("Engine/Shaders/Source/GLSL");
		fileFinder.search_path().emplace_back("Engine/Shaders/Source/GLSL/Includes");
		fileFinder.search_path().emplace_back(ProjectManager::GetAssetsDirectory().string());

		compileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		compileOptions.SetWarningsAsErrors();
		compileOptions.SetIncluder(std::make_unique<glslc::FileIncluder>(&fileFinder));
		compileOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
		compileOptions.AddMacroDefinition("__GLSL__");

#ifdef VT_ENABLE_SHADER_DEBUG
		compileOptions.SetGenerateDebugInfo();
#endif

		std::string processedSource = source;
		if (!PreprocessGLSL(stage, filepath, processedSource, compiler, compileOptions))
		{
			return false;
		}

		// Compile to SPIR-V
		{
			shaderc::SpvCompilationResult compileResult = compiler.CompileGlslToSpv(processedSource, Utility::VulkanToShaderCStage(stage), filepath.string().c_str());
			if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				VT_CORE_ERROR("Failed to compile shader {0}", filepath.string().c_str());
				VT_CORE_ERROR(compileResult.GetErrorMessage().c_str());

				return false;
			}

			const auto* begin = (const uint8_t*)compileResult.cbegin();
			const auto* end = (const uint8_t*)compileResult.cend();
			const ptrdiff_t size = end - begin;

			outShaderData = std::vector<uint32_t>(compileResult.cbegin(), compileResult.cend());
		}

		// Cache Shader
		{
			std::ofstream output{ cachedPath, std::ios::binary | std::ios::out };
			if (!output.is_open())
			{
				VT_CORE_ERROR("Failed to open file {0} for writing!", cachedPath.string().c_str());
			}

			output.write((const char*)outShaderData.data(), outShaderData.size() * sizeof(uint32_t));
			output.close();
		}

		return true;
	}

	bool ShaderCompiler::CompileHLSL(const VkShaderStageFlagBits stage, const std::string& source, const std::filesystem::path filepath, std::vector<uint32_t>& outShaderData)
	{
		if (!DXCInstances::compiler)
		{
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DXCInstances::compiler));
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&DXCInstances::utils));
		}

		std::string processedSource = source;
		if (!PreprocessHLSL(stage, filepath, processedSource))
		{
			return false;
		}

		std::vector<const wchar_t*> arguments =
		{
			filepath.c_str(),
			L"-E",
			L"main",
			L"-T",
			Utility::HLSLShaderProfile(stage),
			L"-spirv",
			L"-fspv-target-env=vulkan1.3",
			L"-fvk-support-nonzero-base-instance",
			L"-HV",
			L"2021",
			DXC_ARG_PACK_MATRIX_COLUMN_MAJOR,
			DXC_ARG_WARNINGS_ARE_ERRORS
		};

#ifdef VT_ENABLE_SHADER_DEBUG
		arguments.emplace_back(DXC_ARG_DEBUG);
		//arguments.emplace_back(L"-fspv-debug=vulkan-with-source"); // https://github.com/microsoft/DirectXShaderCompiler/issues/4979
#endif

		if (stage & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT))
		{
			arguments.emplace_back(L"-fvk-invert-y");
		}

		IDxcBlobEncoding* sourcePtr;
		DXCInstances::utils->CreateBlob(processedSource.c_str(), (uint32_t)processedSource.size(), CP_UTF8, &sourcePtr);

		DxcBuffer sourceBuffer{};
		sourceBuffer.Ptr = sourcePtr->GetBufferPointer();
		sourceBuffer.Size = sourcePtr->GetBufferSize();
		sourceBuffer.Encoding = 0;

		IDxcResult* compilationResult;
		std::string error;
		HRESULT result = DXCInstances::compiler->Compile(&sourceBuffer, arguments.data(), (uint32_t)arguments.size(), nullptr, IID_PPV_ARGS(&compilationResult));

		const bool failed = FAILED(result);
		if (failed)
		{
			error = std::format("Failed to compile. Error: {}\n", result);
			error.append(std::format("{0}\nWhile compiling shader file: {1}", Utility::GetErrorStringFromResult(compilationResult), filepath.string()));
		}

		if (error.empty())
		{
			IDxcBlob* shaderResult;
			compilationResult->GetResult(&shaderResult);

			if (!shaderResult || shaderResult->GetBufferSize() == 0)
			{
				error = std::format("Failed to compile. Error: {}\n", result);
				error.append(std::format("{0}\nWhile compiling shader file: {1}", Utility::GetErrorStringFromResult(compilationResult), filepath.string()));
				VT_CORE_ERROR(error);

				sourcePtr->Release();
				compilationResult->Release();
				return false;
			}

			const size_t size = shaderResult->GetBufferSize();
			outShaderData.resize(size / sizeof(uint32_t));
			memcpy_s(outShaderData.data(), size, shaderResult->GetBufferPointer(), size);
			shaderResult->Release();
		}
		else
		{
			sourcePtr->Release();
			compilationResult->Release();
			VT_CORE_ERROR(error);
			return false;
		}

		// Cache shader
		{
			const auto cacheDirectory = Utility::GetShaderCacheDirectory();
			const auto extension = Utility::GetShaderStageCachedFileExtension(stage);
			const auto cachedPath = cacheDirectory / (filepath.filename().string() + extension);

			std::ofstream output{ cachedPath, std::ios::binary | std::ios::out };
			if (!output.is_open())
			{
				VT_CORE_ERROR("Failed to open file {0} for writing!", cachedPath.string().c_str());
			}

			output.write((const char*)outShaderData.data(), outShaderData.size() * sizeof(uint32_t));
			output.close();
		}

		sourcePtr->Release();
		compilationResult->Release();

		return true;
	}

	bool ShaderCompiler::PreprocessGLSL(const VkShaderStageFlagBits stage, const std::filesystem::path& filepath, std::string& outSource, shaderc::Compiler& compiler, const shaderc::CompileOptions& compileOptions)
	{
		shaderc::PreprocessedSourceCompilationResult preProcessResult = compiler.PreprocessGlsl(outSource, Utility::VulkanToShaderCStage(stage), filepath.string().c_str(), compileOptions);
		if (preProcessResult.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			VT_CORE_ERROR("Failed to preprocess shader {0}!", filepath.string().c_str());
			VT_CORE_ERROR("{0}", preProcessResult.GetErrorMessage().c_str());

			return false;
		}

		outSource = std::string(preProcessResult.cbegin(), preProcessResult.cend());
		return true;
	}

	bool ShaderCompiler::PreprocessHLSL(const VkShaderStageFlagBits stage, const std::filesystem::path& filepath, std::string& outSource)
	{
		const auto assetWStr = L"-I " + ProjectManager::GetAssetsDirectory().wstring();

		std::vector<const wchar_t*> arguments =
		{
			filepath.c_str(),
			L"-P",
			DXC_ARG_WARNINGS_ARE_ERRORS,
			L"-I Engine/Shaders/Source/Includes",
			L"-I Engine/Shaders/Source/HLSL",
			L"-I Engine/Shaders/Source/HLSL/Includes",
			assetWStr.c_str(),
			L"-D", L"__HLSL__"
		};

		IDxcBlobEncoding* sourcePtr;
		DXCInstances::utils->CreateBlob(outSource.c_str(), (uint32_t)outSource.size(), CP_UTF8, &sourcePtr);

		DxcBuffer sourceBuffer{};
		sourceBuffer.Ptr = sourcePtr->GetBufferPointer();
		sourceBuffer.Size = sourcePtr->GetBufferSize();
		sourceBuffer.Encoding = 0;

		const Scope<HLSLIncluder> includer = CreateScope<HLSLIncluder>();
		IDxcResult* compilationResult;
		HRESULT result = DXCInstances::compiler->Compile(&sourceBuffer, arguments.data(), (uint32_t)arguments.size(), includer.get(), IID_PPV_ARGS(&compilationResult));

		std::string error;
		const bool failed = FAILED(result);
		if (failed)
		{
			error = std::format("Failed to compile. Error {0}\n", result);
			IDxcBlobUtf8* errors;
			compilationResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
			if (errors && errors->GetStringLength() > 0)
			{
				error.append(std::format("{0}\nWhile compiling shader file: {1}", (char*)errors->GetBufferPointer(), filepath.string()));
			}
		}

		if (error.empty())
		{
			IDxcBlob* result;
			compilationResult->GetResult(&result);

			outSource = (const char*)result->GetBufferPointer();
			result->Release();
		}

		sourcePtr->Release();
		compilationResult->Release();

		return true;
	}
}
