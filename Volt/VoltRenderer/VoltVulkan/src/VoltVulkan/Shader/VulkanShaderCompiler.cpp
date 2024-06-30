#include "vkpch.h"
#include "VulkanShaderCompiler.h"

#include "VoltVulkan/Shader/VulkanShader.h"
#include "VoltVulkan/Shader/HLSLIncluder.h"
#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Shader/ShaderUtility.h>
#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Shader/ShaderPreProcessor.h>
#include <VoltRHI/Shader/ShaderCache.h>

#include <CoreUtilities/TimeUtility.h>
#include <CoreUtilities/StringUtility.h>

#ifdef _WIN32
#include <wrl.h>
#else
#include <dxc/WinAdapter.h>
#endif

#include <dxc/dxcapi.h>

#include <spirv_cross/spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>

#include <codecvt>
#include <locale>

namespace Volt::RHI
{
	namespace Utility
	{
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

		inline static const ShaderUniformType GetShaderUniformTypeFromSPIRV(const spirv_cross::SPIRType& type)
		{
			ShaderUniformType resultType;

			switch (type.basetype)
			{
				case spirv_cross::SPIRType::Boolean: resultType.baseType = ShaderUniformBaseType::Bool; break;
				case spirv_cross::SPIRType::UInt: resultType.baseType = ShaderUniformBaseType::UInt; break;
				case spirv_cross::SPIRType::Int: resultType.baseType = ShaderUniformBaseType::Int; break;
				case spirv_cross::SPIRType::Float: resultType.baseType = ShaderUniformBaseType::Float; break;
				case spirv_cross::SPIRType::Half: resultType.baseType = ShaderUniformBaseType::Half; break;
				case spirv_cross::SPIRType::Short: resultType.baseType = ShaderUniformBaseType::Short; break;
				case spirv_cross::SPIRType::UShort: resultType.baseType = ShaderUniformBaseType::UShort; break;
			}

			resultType.columns = type.columns;
			resultType.vecsize = type.vecsize;

			return resultType;
		}
	}

	VulkanShaderCompiler::VulkanShaderCompiler(const ShaderCompilerCreateInfo& createInfo)
		: m_includeDirectories(createInfo.includeDirectories), m_macros(createInfo.initialMacros), m_flags(createInfo.flags)
	{
		RHILog::LogTagged(LogSeverity::Trace, "[VulkanShaderCompiler]", "Initializing VulkanShaderCompiler");
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_hlslCompiler));
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_hlslUtils));
	}

	VulkanShaderCompiler::~VulkanShaderCompiler()
	{
		m_hlslUtils->Release();
		m_hlslCompiler->Release();

		RHILog::LogTagged(LogSeverity::Trace, "[VulkanShaderCompiler]", "Destroying VulkanShaderCompiler");
	}

	ShaderCompiler::CompilationResultData VulkanShaderCompiler::TryCompileImpl(const Specification& specification)
	{
		// First try and get cached shader
		if (!specification.forceCompile)
		{
			const auto cachedResult = ShaderCache::TryGetCachedShader(specification);
			if (cachedResult.data.IsValid())
			{
				return cachedResult.data;
			}
		}

		if (specification.shaderSourceInfo.empty())
		{
			RHILog::LogTagged(LogSeverity::Error, "[VulkanShaderCompiler]", "Trying to compile a shader without sources!");
			return {};
		}

		CompilationResultData result = CompileAll(specification);

		// If compilation fails, we try to get the cached version.
		if (result.result != ShaderCompiler::CompilationResult::Success)
		{
			const auto cachedResult = ShaderCache::TryGetCachedShader(specification);
			return cachedResult.data;
		}

		ReflectAllStages(specification, result);
		ShaderCache::CacheShader(specification, result);

		return result;
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

	ShaderCompiler::CompilationResultData VulkanShaderCompiler::CompileAll(const Specification& specification)
	{
		CompilationResultData result;

		for (const auto& [stage, sourceInfo] : specification.shaderSourceInfo)
		{
			result.result = CompileSingle(stage, sourceInfo.source, sourceInfo.sourceEntry, specification, result);

			if (result.result != ShaderCompiler::CompilationResult::Success)
			{
				break;
			}
		}

		// #TODO_Ivar: This is a dumb hack for vertex only shaders
		if (specification.shaderSourceInfo.contains(RHI::ShaderStage::Vertex) && !specification.shaderSourceInfo.contains(RHI::ShaderStage::Pixel))
		{
			result.outputFormats.emplace_back(RHI::PixelFormat::D32_SFLOAT);
		}

		return result;
	}

	ShaderCompiler::CompilationResult VulkanShaderCompiler::CompileSingle(ShaderStage shaderStage, const std::string& source, const ShaderSourceEntry& sourceEntry, const Specification& specification, CompilationResultData& outData)
	{
		auto& data = outData.shaderData[shaderStage];

		std::string processedSource = source;

		if (!PreprocessSource(shaderStage, sourceEntry.filePath, processedSource))
		{
			return CompilationResult::PreprocessFailed;
		}

		const std::wstring wEntryPoint = ::Utility::ToWString(sourceEntry.entryPoint);

		std::vector<const wchar_t*> arguments =
		{
			sourceEntry.filePath.c_str(),
			L"-E",
			wEntryPoint.c_str(),
			L"-T",
			Utility::HLSLShaderProfile(shaderStage),
			L"-spirv",
			L"-fspv-target-env=vulkan1.3",
			L"-HV",
			L"2021",
			L"-D",
			L"__VULKAN__ ",
			L"-enable-16bit-types",
			L"-fvk-use-scalar-layout",

			DXC_ARG_PACK_MATRIX_COLUMN_MAJOR
		};

		if ((m_flags & ShaderCompilerFlags::WarningsAsErrors) != ShaderCompilerFlags::None)
		{
			arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
		}

		switch (specification.optimizationLevel)
		{
			case ShaderCompiler::OptimizationLevel::Disable: arguments.push_back(L"-Od"); break;
			case ShaderCompiler::OptimizationLevel::Release: arguments.push_back(L"-O1"); break;
			case ShaderCompiler::OptimizationLevel::Dist: arguments.push_back(L"-O3"); break;
		}

		if (specification.optimizationLevel != ShaderCompiler::OptimizationLevel::Dist)
		{
			arguments.push_back(DXC_ARG_DEBUG);
			/*arguments.push_back(L"-fspv-debug=vulkan");*/
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
			processingData.entryPoint = sourceEntry.entryPoint;

			PreProcessorResult result{};
			if (!ShaderPreProcessor::PreProcessShaderSource(processingData, result))
			{
				return CompilationResult::PreprocessFailed;
			}

			if (shaderStage == ShaderStage::Pixel)
			{
				outData.outputFormats = result.outputFormats;
			}
			else if (shaderStage == ShaderStage::Vertex)
			{
				outData.vertexLayout = result.vertexLayout;
				outData.instanceLayout = result.instanceLayout;
			}

			if (!outData.renderGraphConstants.IsValid())
			{
				outData.renderGraphConstants = result.renderGraphConstants;
			}
			else if (result.renderGraphConstants.IsValid())
			{
				for (const auto& [name, uniform] : outData.renderGraphConstants.uniforms)
				{
					if (!result.renderGraphConstants.uniforms.contains(name) || uniform.type != result.renderGraphConstants.uniforms.at(name).type)
					{
						RHILog::LogTagged(LogSeverity::Error, "[VulkanShaderCompiler]", "All shader stages must have equal constant struct definition!");
					}
				}
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
			error.append(std::format("{0}\nWhile compiling shader file: {1}", Utility::GetErrorStringFromResult(compilationResult), sourceEntry.filePath.string()));
		}

		if (error.empty())
		{
			IDxcBlob* shaderResult = nullptr;
			compilationResult->GetResult(&shaderResult);

			if (!shaderResult || shaderResult->GetBufferSize() == 0)
			{
				error = std::format("Failed to compile. Error: {}\n", result);
				error.append(std::format("{0}\nWhile compiling shader file: {1}", Utility::GetErrorStringFromResult(compilationResult), sourceEntry.filePath.string()));

				RHILog::LogUnformatted(LogSeverity::Error, "[VulkanShaderCompiler]: " + error);

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

			RHILog::LogUnformatted(LogSeverity::Error, "[VulkanShaderCompiler]: " + error);
			return CompilationResult::Failure;
		}

		sourcePtr->Release();
		compilationResult->Release();

		return CompilationResult::Success;
	}

	void VulkanShaderCompiler::ReflectAllStages(const Specification& specification, CompilationResultData& inOutData)
	{
		for (const auto& [stage, data] : inOutData.shaderData)
		{
			RHILog::LogTagged(LogSeverity::Trace, "[VulkanShaderCompiler]", "Reflecting shader {0}", specification.shaderSourceInfo.at(stage).sourceEntry.filePath.string());
			ReflectStage(stage, specification, inOutData);
		}
	}

	void VulkanShaderCompiler::ReflectStage(ShaderStage stage, const Specification& specification, CompilationResultData& inOutData)
	{
		spirv_cross::Compiler compiler{ inOutData.shaderData[stage] };
		const auto resources = compiler.get_shader_resources();

		for (const auto& ubo : resources.uniform_buffers)
		{
			if (compiler.get_active_buffer_ranges(ubo.id).empty())
			{
				continue;
			}

			const auto& bufferType = compiler.get_type(ubo.base_type_id);

			const size_t size = compiler.get_declared_struct_size(bufferType);
			const uint32_t binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
			const std::string& name = compiler.get_name(ubo.id);

			if (!TryAddShaderBinding(name, set, binding, inOutData))
			{
				RHILog::LogTagged(LogSeverity::Error, "[VulkanShaderCompiler]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			if (name == "$Globals")
			{
				RHILog::LogTagged(LogSeverity::Error, "[VulkanShaderCompiler]", "Shader {0} seems to have incorrectly defined global variables!", specification.shaderSourceInfo.at(stage).sourceEntry.filePath.string());
				continue;
			}

			auto& buffer = inOutData.uniformBuffers[set][binding];
			buffer.usageStages = buffer.usageStages | stage;
			buffer.usageCount++;
			buffer.size = size;
		}

		for (const auto& ssbo : resources.storage_buffers)
		{
			const auto& bufferBaseType = compiler.get_type(ssbo.base_type_id);
			const auto& bufferType = compiler.get_type(ssbo.type_id);

			const size_t size = compiler.get_declared_struct_size(bufferBaseType);
			const uint32_t binding = compiler.get_decoration(ssbo.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(ssbo.id, spv::DecorationDescriptorSet);
			const std::string& name = compiler.get_name(ssbo.id);

			if (!TryAddShaderBinding(name, set, binding, inOutData))
			{
				RHILog::LogTagged(LogSeverity::Error, "[VulkanShaderCompiler]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			const bool firstEntry = !inOutData.storageBuffers[set].contains(binding);

			auto& buffer = inOutData.storageBuffers[set][binding];
			buffer.usageStages = buffer.usageStages | stage;
			buffer.usageCount++;
			buffer.size = size;

			if (firstEntry && !bufferType.array.empty())
			{
				const int32_t arraySize = static_cast<int32_t>(bufferType.array[0]);

				if (arraySize == 0)
				{
					buffer.arraySize = -1;
				}
				else
				{
					buffer.arraySize = arraySize;
				}
			}
		}

		for (const auto& image : resources.storage_images)
		{
			const uint32_t binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			const auto& imageType = compiler.get_type(image.type_id);
			const std::string& name = compiler.get_name(image.id);

			if (!TryAddShaderBinding(name, set, binding, inOutData))
			{
				RHILog::LogTagged(LogSeverity::Error, "[VulkanShader]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			const bool firstEntry = !inOutData.storageImages[set].contains(binding);

			auto& shaderImage = inOutData.storageImages[set][binding];
			shaderImage.usageStages = shaderImage.usageStages | stage;
			shaderImage.usageCount++;

			if (firstEntry && !imageType.array.empty())
			{
				const int32_t arraySize = static_cast<int32_t>(imageType.array[0]);

				if (arraySize == 0)
				{
					shaderImage.arraySize = -1;
				}
				else
				{
					shaderImage.arraySize = arraySize;
				}
			}
		}

		for (const auto& image : resources.separate_images)
		{
			const uint32_t binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			const auto& imageType = compiler.get_type(image.type_id);
			const std::string& name = compiler.get_name(image.id);

			if (!TryAddShaderBinding(name, set, binding, inOutData))
			{
				RHILog::LogTagged(LogSeverity::Error, "[VulkanShader]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			const bool firstEntry = !inOutData.images[set].contains(binding);

			auto& shaderImage = inOutData.images[set][binding];
			shaderImage.usageStages = shaderImage.usageStages | stage;
			shaderImage.usageCount++;

			if (firstEntry && !imageType.array.empty())
			{
				const int32_t arraySize = static_cast<int32_t>(imageType.array[0]);

				if (arraySize == 0)
				{
					shaderImage.arraySize = -1;
				}
				else
				{
					shaderImage.arraySize = arraySize;
				}
			}
		}

		for (const auto& sampler : resources.separate_samplers)
		{
			const uint32_t binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
			const uint32_t set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);
			const std::string& name = compiler.get_name(sampler.id);

			if (!TryAddShaderBinding(name, set, binding, inOutData))
			{
				RHILog::LogTagged(LogSeverity::Error, "[VulkanShader]", "Unable to add binding with name {0} to list. It already exists!", name);
			}

			auto& shaderSampler = inOutData.samplers[set][binding];
			shaderSampler.usageStages = shaderSampler.usageStages | stage;
			shaderSampler.usageCount++;
		}

		for (const auto& pushConstant : resources.push_constant_buffers)
		{
			const auto& bufferType = compiler.get_type(pushConstant.base_type_id);
			const size_t pushConstantSize = compiler.get_declared_struct_size(bufferType);

			inOutData.constantsBuffer.SetSize(pushConstantSize);

			inOutData.constants.size = static_cast<uint32_t>(pushConstantSize);
			inOutData.constants.offset = 0;
			inOutData.constants.stageFlags = inOutData.constants.stageFlags | stage;

			for (uint32_t i = 0; const auto & member : bufferType.member_types)
			{
				const auto& memberType = compiler.get_type(member);
				const size_t memberSize = compiler.get_declared_struct_member_size(bufferType, i);
				const size_t memberOffset = compiler.type_struct_member_offset(bufferType, i);
				const std::string& memberName = compiler.get_member_name(pushConstant.base_type_id, i);

				const auto type = Utility::GetShaderUniformTypeFromSPIRV(memberType);

				inOutData.constantsBuffer.AddMember(memberName, type, memberSize, memberOffset);
				i++;
			}
		}
	}

	bool VulkanShaderCompiler::TryAddShaderBinding(const std::string& name, uint32_t set, uint32_t binding, CompilationResultData& outData)
	{
		if (outData.bindings.contains(name))
		{
			return false;
		}

		outData.bindings[name] = { set, binding };
		return true;
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
			L"-D", L"__HLSL__",
			L"-D", L"__VULKAN__"
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
			wMacros.push_back(::Utility::ToWString(macro));
		}

		if ((m_flags & ShaderCompilerFlags::EnableShaderValidator) != ShaderCompilerFlags::None)
		{
			wMacros.push_back(L"ENABLE_RUNTIME_VALIDATION");
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

		const Scope<HLSLIncluder> includer = CreateScope<HLSLIncluder>();

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
			RHILog::LogTagged(LogSeverity::Error, "[VulkanShaderCompiler]", error);
		}

		sourcePtr->Release();
		compilationResult->Release();

		return !failed;
	}

	void* VulkanShaderCompiler::GetHandleImpl() const
	{
		return nullptr;
	}
}
