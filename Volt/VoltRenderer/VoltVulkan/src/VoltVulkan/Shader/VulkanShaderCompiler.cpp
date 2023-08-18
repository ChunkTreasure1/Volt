#include "vkpch.h"
#include "VulkanShaderCompiler.h"

#include "VoltVulkan/Shader/VulkanShader.h"
#include "VoltVulkan/Shader/HLSLIncluder.h"
#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Shader/ShaderUtility.h>
#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Shader/ShaderPreProcessor.h>

#ifdef _WIN32
#include <wrl.h>
#else
#include <dxc/WinAdapter.h>
#endif

#include <dxc/dxcapi.h>

#include <codecvt>
#include <locale>

namespace Volt::RHI
{
	namespace Utility
	{
#pragma warning( push )
#pragma warning( disable : 4996 )
		inline static std::wstring ToWString(const std::string& stringToConvert)
		{
			std::wstring wideString =
				std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(stringToConvert);
			return wideString;
		}
#pragma warning( pop )

		inline static const std::string GetErrorStringFromResult(IDxcResult* result)
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

	VulkanShaderCompiler::VulkanShaderCompiler(const ShaderCompilerCreateInfo& createInfo)
		: m_includeDirectories(createInfo.includeDirectories), m_macros(createInfo.initialMacros), m_flags(createInfo.flags), m_cacheDirectory(createInfo.cacheDirectory)
	{
		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShaderCompiler]", "Initializing VulkanShaderCompiler");
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_hlslCompiler));
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_hlslUtils));
	}

	VulkanShaderCompiler::~VulkanShaderCompiler()
	{
		m_hlslUtils->Release();
		m_hlslCompiler->Release();

		GraphicsContext::LogTagged(Severity::Trace, "[VulkanShaderCompiler]", "Destroying VulkanShaderCompiler");
	}

	ShaderCompiler::CompilationResult VulkanShaderCompiler::TryCompileImpl(const Specification& specification, Shader& shader)
	{
		return CompileAll(specification, shader);
	}

	void VulkanShaderCompiler::AddMacroImpl(const std::string& macroName)
	{
		if (std::find(m_macros.begin(), m_macros.end(), macroName) != m_macros.end())
		{
			return;
		}

		m_macros.push_back(macroName);
	}

	void VulkanShaderCompiler::RemoveMacroImpl(std::string_view macroName)
	{
		if (auto it = std::find(m_macros.begin(), m_macros.end(), macroName); it != m_macros.end())
		{
			m_macros.erase(it);
		}
	}

	ShaderCompiler::CompilationResult VulkanShaderCompiler::CompileAll(const Specification& specification, Shader& shader)
	{
		VulkanShader& vulkanShader = shader.AsRef<VulkanShader>();

		for (const auto& [stage, sourceData] : vulkanShader.m_shaderSources)
		{
			if (CompileSingle(stage, sourceData.source, sourceData.filepath, specification, shader) != CompilationResult::Success)
			{
				return CompilationResult::Failure;
			}
		}

		return CompilationResult::Success;
	}

	ShaderCompiler::CompilationResult VulkanShaderCompiler::CompileSingle(ShaderStage shaderStage, const std::string& source, const std::filesystem::path& filepath, const Specification& specification, Shader& shader)
	{
		auto& vulkanShader = shader.AsRef<VulkanShader>();
		auto& data = vulkanShader.m_shaderData[shaderStage];

		const auto cacheDirectory = m_cacheDirectory / Utility::GetShaderCacheSubDirectory();
		const auto extension = Utility::GetShaderStageCachedFileExtension(shaderStage);
		const auto cachedPath = cacheDirectory / (filepath.filename().string() + extension);

		if (!specification.forceCompile)
		{
			std::ifstream inStream{ cachedPath, std::ios::binary | std::ios::in | std::ios::ate };
			if (inStream.is_open())
			{
				const uint64_t size = inStream.tellg();
				data.resize(size / sizeof(uint32_t)); // We store the data as 4 byte blocks

				inStream.seekg(0, std::ios::beg);
				inStream.read(reinterpret_cast<char*>(data.data()), size);
				inStream.close();
			}
		}

		if (!data.empty())
		{
			return CompilationResult::Success;
		}

		std::string processedSource = source;

		if (!PreprocessSource(shaderStage, filepath, processedSource))
		{
			return CompilationResult::PreprocessFailed;
		}

		const std::wstring wEntryPoint = Utility::ToWString(specification.entryPoint);

		std::vector<const wchar_t*> arguments =
		{
			filepath.c_str(),
			L"-E",
			wEntryPoint.c_str(),
			L"-T",
			Utility::HLSLShaderProfile(shaderStage),
			L"-spirv",
			L"-fspv-target-env=vulkan1.3",
			//L"-fvk-support-nonzero-base-instance",
			L"-HV",
			L"2021",
			L"__VULKAN__"
			//L"-fvk-t-shift", std::to_wstring(VulkanDefaults::ShaderTRegisterOffset), L"0",
			//L"-fvk-u-shift", std::to_wstring(VulkanDefaults::ShaderURegisterOffset), L"0",

			//L"-fvk-b-shift 100 1",
			//L"-fvk-t-shift 200 1",
			//L"-fvk-u-shift 300 1",

			//L"-fvk-b-shift 100 2",
			//L"-fvk-t-shift 200 2",
			//L"-fvk-u-shift 300 2",

			//L"-fvk-b-shift 100 3",
			//L"-fvk-t-shift 200 3",
			//L"-fvk-u-shift 300 3",

			DXC_ARG_PACK_MATRIX_COLUMN_MAJOR
		};

		if ((m_flags & ShaderCompilerFlags::WarningsAsErrors) != ShaderCompilerFlags::None)
		{
			arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
		}

		switch (specification.optimizationLevel)
		{
			case ShaderCompiler::OptimizationLevel::Disable: arguments.push_back(L"-Od"); break;
			case ShaderCompiler::OptimizationLevel::Debug: arguments.push_back(L"-O1"); arguments.push_back(DXC_ARG_DEBUG); break;
			case ShaderCompiler::OptimizationLevel::Full: arguments.push_back(L"-O3"); break;
		}

		if (shaderStage == ShaderStage::Vertex || shaderStage == ShaderStage::Hull || shaderStage == ShaderStage::Geometry)
		{
			arguments.emplace_back(L"-fvk-invert-y");
		}

		// Pre processing
		{
			PreProcessorData processingData{};
			processingData.shaderSource = processedSource;
			processingData.shaderStage = shaderStage;

			PreProcessorResult result{};
			if (!ShaderPreProcessor::PreProcessShaderSource(processingData, result))
			{
				return CompilationResult::PreprocessFailed;
			}

			if (shaderStage == ShaderStage::Pixel)
			{
				vulkanShader.m_resources.outputFormats = result.outputFormats;
			}
			else if (shaderStage == ShaderStage::Vertex)
			{
				vulkanShader.m_resources.vertexLayout = result.vertexLayout;
			}

			processedSource = result.preProcessedResult;
		}

		IDxcBlobEncoding* sourcePtr = nullptr;
		m_hlslUtils->CreateBlob(processedSource.c_str(), static_cast<uint32_t>(processedSource.size()), CP_UTF8, &sourcePtr);

		DxcBuffer sourceBuffer{};
		sourceBuffer.Ptr = sourcePtr->GetBufferPointer();
		sourceBuffer.Size = sourcePtr->GetBufferSize();
		sourceBuffer.Encoding = 0;

		IDxcResult* compilationResult = nullptr;
		std::string error;

		HRESULT result = m_hlslCompiler->Compile(&sourceBuffer, arguments.data(), static_cast<uint32_t>(arguments.size()), nullptr, IID_PPV_ARGS(&compilationResult));

		const bool failed = FAILED(result);
		if (failed)
		{
			error = std::format("Failed to compile. Error: {}\n", result);
			error.append(std::format("{0}\nWhile compiling shader file: {1}", Utility::GetErrorStringFromResult(compilationResult), filepath.string()));
		}

		if (error.empty())
		{
			IDxcBlob* shaderResult = nullptr;
			compilationResult->GetResult(&shaderResult);

			if (!shaderResult || shaderResult->GetBufferSize() == 0)
			{
				error = std::format("Failed to compile. Error: {}\n", result);
				error.append(std::format("{0}\nWhile compiling shader file: {1}", Utility::GetErrorStringFromResult(compilationResult), filepath.string()));

				GraphicsContext::LogTagged(Severity::Error, "[VulkanShaderCompiler]", error);

				sourcePtr->Release();
				compilationResult->Release();

				return CompilationResult::Failure;
			}

			const size_t size = shaderResult->GetBufferSize();

			data.resize(size / sizeof(uint32_t));
			memcpy_s(data.data(), size, shaderResult->GetBufferPointer(), shaderResult->GetBufferSize());

			shaderResult->Release();
		}
		else
		{
			sourcePtr->Release();
			compilationResult->Release();

			GraphicsContext::LogTagged(Severity::Error, "[VulkanShaderCompiler]", error);
			return CompilationResult::Failure;
		}

		// Cache shader
		{
			// Create the required directories if they do not exits
			if (!std::filesystem::exists(cachedPath.parent_path()))
			{
				std::filesystem::create_directories(cachedPath.parent_path());
			}

			std::ofstream output{ cachedPath, std::ios::binary | std::ios::out };
			if (!output.is_open())
			{
				GraphicsContext::LogTagged(Severity::Error, "[VulkanShaderCompiler]", "Unable to open path {0} for writing!", cachedPath.string());
			}

			output.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint32_t));
			output.close();
		}

		sourcePtr->Release();
		compilationResult->Release();

		return CompilationResult::Success;
	}

	bool VulkanShaderCompiler::PreprocessSource(const ShaderStage shaderStage, const std::filesystem::path& filepath, std::string& outSource)
	{
		std::vector<std::wstring> wIncludeDirs;
		std::vector<const wchar_t*> wcIncludeDirs;

		for (const auto& includeDir : m_includeDirectories)
		{
			wIncludeDirs.push_back(L"-I " + includeDir.wstring());
		}

		for (const auto& includeDir : wIncludeDirs)
		{
			wcIncludeDirs.push_back(includeDir.c_str());
		}

		std::vector<const wchar_t*> arguments =
		{
			filepath.c_str(),
			L"-P", // Preproccess
			L"-D", L"__HLSL__", L"__VULKAN__"
		};

		if ((m_flags & ShaderCompilerFlags::WarningsAsErrors) != ShaderCompilerFlags::None)
		{
			arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
		}

		for (const auto& includeDir : wcIncludeDirs)
		{
			arguments.push_back(includeDir);
		}

		std::vector<std::wstring> wMacros;
		for (const auto& macro : m_macros)
		{
			wMacros.push_back(Utility::ToWString(macro));
		}

		for (const auto& macro : wMacros)
		{
			arguments.emplace_back(L"-D");
			arguments.emplace_back(macro.c_str());
		}

		IDxcBlobEncoding* sourcePtr = nullptr;
		m_hlslUtils->CreateBlob(outSource.c_str(), static_cast<uint32_t>(outSource.size()), CP_UTF8, &sourcePtr);

		DxcBuffer sourceBuffer{};
		sourceBuffer.Ptr = sourcePtr->GetBufferPointer();
		sourceBuffer.Size = sourcePtr->GetBufferSize();
		sourceBuffer.Encoding = 0;

		const Scope<HLSLIncluder> includer = CreateScopeRHI<HLSLIncluder>();

		IDxcResult* compilationResult = nullptr;
		HRESULT result = m_hlslCompiler->Compile(&sourceBuffer, arguments.data(), static_cast<uint32_t>(arguments.size()), includer.get(), IID_PPV_ARGS(&compilationResult));

		std::string error;
		const bool failed = FAILED(result);

		if (failed)
		{
			error = std::format("Failed to compile. Error {0}\n", result);
			error.append(std::format("{0}\nWhile compiling shader file: {1}", Utility::GetErrorStringFromResult(compilationResult), filepath.string()));
		}

		if (error.empty())
		{
			IDxcBlob* compileResult = nullptr;
			compilationResult->GetResult(&compileResult);

			outSource = reinterpret_cast<const char*>(compileResult->GetBufferPointer());
			compileResult->Release();
		}
		else
		{
			GraphicsContext::LogTagged(Severity::Error, "[VulkanShaderCompiler]", error);
		}

		sourcePtr->Release();
		compilationResult->Release();

		return !failed;
	}

	void* VulkanShaderCompiler::GetHandleImpl()
	{
		return nullptr;
	}
}
