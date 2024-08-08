#include "dxpch.h"
#include "D3D12ShaderCompiler.h"

#include "D3D12RHIModule/Shader/D3D12Shader.h"
#include "D3D12RHIModule/Shader/HLSLIncluder.h"

#include <RHIModule/Shader/ShaderUtility.h>
#include <RHIModule/Shader/ShaderPreProcessor.h>
#include <RHIModule/Shader/ShaderCache.h>
#include <RHIModule/Globals.h>

#include <CoreUtilities/StringUtility.h>

#include <dxsc/dxcapi.h>
#include <d3d12/d3d12shader.h>

#include <ranges>
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

		inline ShaderUniformType GetShaderUniformTypeFromD3D12(ID3D12ShaderReflectionType* type)
		{
			D3D12_SHADER_TYPE_DESC desc{};
			type->GetDesc(&desc);

			ShaderUniformType resultType;

			switch (desc.Type)
			{
				case D3D_SVT_BOOL: resultType.baseType = ShaderUniformBaseType::Bool; break;
				case D3D_SVT_UINT: resultType.baseType = ShaderUniformBaseType::UInt; break;
				case D3D_SVT_INT: resultType.baseType = ShaderUniformBaseType::Int; break;
				case D3D_SVT_FLOAT: resultType.baseType = ShaderUniformBaseType::Float; break;
				case D3D_SVT_FLOAT16: resultType.baseType = ShaderUniformBaseType::Half; break;
				case D3D_SVT_INT16: resultType.baseType = ShaderUniformBaseType::Short; break;
				case D3D_SVT_UINT16: resultType.baseType = ShaderUniformBaseType::UShort; break;
			}

			// #TODO_Ivar: Is this correct?
			resultType.columns = desc.Rows;
			resultType.vecsize = desc.Columns;
		
			return resultType;
		}
	}

	D3D12ShaderCompiler::D3D12ShaderCompiler(const ShaderCompilerCreateInfo& createInfo) 
		: m_includeDirectories(createInfo.includeDirectories), m_macros(createInfo.initialMacros), m_flags(createInfo.flags)
	{
		VT_LOGC(Trace, LogD3D12RHI, "Initializing D3D12ShaderCompiler");
		DxcCreateInstance(CLSID_DxcUtils, VT_D3D12_ID(m_hlslUtils));
		DxcCreateInstance(CLSID_DxcCompiler, VT_D3D12_ID(m_hlslCompiler));
	}

	D3D12ShaderCompiler::~D3D12ShaderCompiler()
	{
		m_hlslUtils->Release();
		m_hlslCompiler->Release();

		VT_LOGC(Trace, LogD3D12RHI, "Destroying D3D12ShaderCompiler");
	}

	void* D3D12ShaderCompiler::GetHandleImpl() const
	{
		return nullptr;
	}

	ShaderCompiler::CompilationResultData D3D12ShaderCompiler::TryCompileImpl(const Specification& specification)
	{
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
			VT_LOGC(Error, LogD3D12RHI, "Trying to compile a shader without sources!");
			return {};
		}

		std::unordered_map<ShaderStage, ID3D12ShaderReflection*> reflectionData;
		CompilationResultData result = CompileAll(specification, reflectionData);
		
		// If compilation fails, we try to get the cached version.
		if (result.result != ShaderCompiler::CompilationResult::Success)
		{
			const auto cachedResult = ShaderCache::TryGetCachedShader(specification);
			return cachedResult.data;
		}

		ReflectAllStages(specification, result, reflectionData);
		ShaderCache::CacheShader(specification, result);

		return result;
	}

	void D3D12ShaderCompiler::AddMacroImpl(const std::string& macroName)
	{
		if (std::find(m_macros.begin(), m_macros.end(), macroName) != m_macros.end())
		{
			return;
		}

		m_macros.push_back(macroName);
	}

	void D3D12ShaderCompiler::RemoveMacroImpl(std::string_view macroName)
	{
		if (auto it = std::find(m_macros.begin(), m_macros.end(), macroName); it != m_macros.end())
		{
			m_macros.erase(it);
		}
	}

	bool D3D12ShaderCompiler::PreprocessSource(const ShaderStage shaderStage, const std::filesystem::path& filepath, std::string& outSource)
	{
		Vector<std::wstring> wIncludeDirs;
		Vector<const wchar_t*> wcIncludeDirs;

		for (const auto& includeDir : m_includeDirectories)
		{
			wIncludeDirs.push_back(L"-I " + includeDir.wstring());
		}

		for (const auto& includeDir : wIncludeDirs)
		{
			wcIncludeDirs.push_back(includeDir.c_str());
		}

		Vector<const wchar_t*> arguments =
		{
			filepath.c_str(),
			L"-P", // Preproccess
			L"-D", L"__HLSL__",
			L"-D", L"__D3D12__"
		};

		if ((m_flags & ShaderCompilerFlags::WarningsAsErrors) != ShaderCompilerFlags::None)
		{
			arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
		}

		for (const auto& includeDir : wcIncludeDirs)
		{
			arguments.push_back(includeDir);
		}

		Vector<std::wstring> wMacros;
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
			VT_LOGC(Error, LogD3D12RHI, error);
		}

		sourcePtr->Release();
		compilationResult->Release();

		return !failed;
	}

	ShaderCompiler::CompilationResultData D3D12ShaderCompiler::CompileAll(const Specification& specification, std::unordered_map<ShaderStage, ID3D12ShaderReflection*>& outReflectionData)
	{
		CompilationResultData result;

		for (const auto& [stage, sourceInfo] : specification.shaderSourceInfo)
		{
			result.result = CompileSingle(stage, sourceInfo.source, sourceInfo.sourceEntry, specification, result, &outReflectionData[stage]);

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


	ShaderCompiler::CompilationResult D3D12ShaderCompiler::CompileSingle(ShaderStage shaderStage, const std::string& source, const ShaderSourceEntry& sourceEntry, const Specification& specification, CompilationResultData& outData, ID3D12ShaderReflection** outReflectionData)
	{
		auto& data = outData.shaderData[shaderStage];

		std::string processedSource = source;

		if (!PreprocessSource(shaderStage, sourceEntry.filePath, processedSource))
		{
			return CompilationResult::PreprocessFailed;
		}

		const std::wstring wEntryPoint = ::Utility::ToWString(sourceEntry.entryPoint);

		Vector<const wchar_t*> arguments =
		{
			sourceEntry.filePath.c_str(),
			L"-E",
			wEntryPoint.c_str(),
			L"-T",
			Utility::HLSLShaderProfile(shaderStage),
			L"-HV",
			L"2021",
			L"-D",
			L"__D3D12__ ",
			L"-enable-16bit-types",

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
			arguments.push_back(L"-Qembed_debug");
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
						VT_LOGC(Error, LogD3D12RHI, "All shader stages must have equal constant struct definition!");
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

				VT_LOG(Error, "[D3D12ShaderCompiler]: " + error);

				sourcePtr->Release();
				compilationResult->Release();

				return CompilationResult::Failure;
			}

			const size_t size = shaderResult->GetBufferSize();

			data.resize(size / sizeof(uint32_t));
			memcpy_s(data.data(), size, shaderResult->GetBufferPointer(), shaderResult->GetBufferSize());

			// Get reflection data
			IDxcBlob* pReflectionData = nullptr;
			compilationResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);

			DxcBuffer reflectionBuffer{};
			if (pReflectionData)
			{
				reflectionBuffer.Ptr = pReflectionData->GetBufferPointer();
				reflectionBuffer.Size = pReflectionData->GetBufferSize();
				reflectionBuffer.Encoding = 0;

				m_hlslUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(outReflectionData));
				pReflectionData->Release();
			}
			else
			{
				error = std::format("Failed to compile. Error: {}\n", result);
				error.append(std::format("{0}\nWhile compiling shader file: {1}", Utility::GetErrorStringFromResult(compilationResult), sourceEntry.filePath.string()));

				VT_LOG(Error, "[D3D12ShaderCompiler]: " + error);
			}

			compilationResult->Release();
			shaderResult->Release();
		}
		else
		{
			sourcePtr->Release();
			compilationResult->Release();

			VT_LOG(Error, "[D3D12ShaderCompiler]: " + error);
			return CompilationResult::Failure;
		}

		sourcePtr->Release();
		compilationResult->Release();

		return CompilationResult::Success;
	}

	void D3D12ShaderCompiler::ReflectAllStages(const Specification& specification, CompilationResultData& inOutData, const std::unordered_map<ShaderStage, ID3D12ShaderReflection*>& reflectionData)
	{
		for (const auto& [stage, data] : inOutData.shaderData)
		{
			if (!reflectionData.at(stage))
			{
				VT_LOGC(Warning, LogD3D12RHI, "No reflection data availiable for shader {}!", specification.shaderSourceInfo.at(stage).sourceEntry.filePath.string());
				continue;
			}

			VT_LOGC(Trace, LogD3D12RHI, "Reflecting shader {0}", specification.shaderSourceInfo.at(stage).sourceEntry.filePath.string());
			ReflectStage(stage, specification, inOutData, reflectionData.at(stage));
		}
	}

	void D3D12ShaderCompiler::ReflectStage(ShaderStage stage, const Specification& specification, CompilationResultData& inOutData, ID3D12ShaderReflection* reflectionData)
	{
		D3D12_SHADER_DESC shaderDesc{};
		reflectionData->GetDesc(&shaderDesc);

		for (uint32_t i : std::views::iota(0u, shaderDesc.BoundResources))
		{
			D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{};
			VT_D3D12_CHECK(reflectionData->GetResourceBindingDesc(i, &shaderInputBindDesc));

			const uint32_t binding = shaderInputBindDesc.BindPoint;
			const uint32_t space = shaderInputBindDesc.Space;
			const std::string name = shaderInputBindDesc.Name;

			ShaderRegisterType registerType = ShaderRegisterType::Texture;

			if (name == "$Globals")
			{
				VT_LOGC(Error, LogD3D12RHI, "Shader {0} seems to have incorrectly defined global variables!", specification.shaderSourceInfo.at(stage).sourceEntry.filePath.string());
				continue;
			}

			if (shaderInputBindDesc.Type == D3D_SIT_CBUFFER)
			{
				ID3D12ShaderReflectionConstantBuffer* reflectedCB = reflectionData->GetConstantBufferByIndex(i);
				D3D12_SHADER_BUFFER_DESC cbDesc{};
				reflectedCB->GetDesc(&cbDesc);

				registerType = ShaderRegisterType::UniformBuffer;

				const size_t size = static_cast<size_t>(cbDesc.Size);

				if (binding == Globals::PUSH_CONSTANTS_BINDING)
				{
					inOutData.constantsBuffer.SetSize(size);
					inOutData.constants.size = static_cast<uint32_t>(size);

					ID3D12ShaderReflectionVariable* baseVariable = reflectedCB->GetVariableByIndex(0);
					ID3D12ShaderReflectionType* baseType = baseVariable->GetType();
					D3D12_SHADER_TYPE_DESC baseTypeDesc;
					baseType->GetDesc(&baseTypeDesc);

					D3D12_SHADER_VARIABLE_DESC varDesc{};
					baseVariable->GetDesc(&varDesc);

					if (baseTypeDesc.Members == 0)
					{
						const uint32_t memberSize = varDesc.Size;
						const uint32_t memberOffset = varDesc.StartOffset;
						const std::string memberName = varDesc.Name;

						const auto type = Utility::GetShaderUniformTypeFromD3D12(baseType);

						inOutData.constantsBuffer.AddMember(memberName, type, memberSize, memberOffset);
					}
					else
					{
						for (uint32_t k = 0; k < baseTypeDesc.Members; k++)
						{

							ID3D12ShaderReflectionType* memberType = baseType->GetMemberTypeByIndex(k);
							D3D12_SHADER_TYPE_DESC memberDesc{};
							memberType->GetDesc(&memberDesc);

							const uint32_t memberSize = 0;
							const uint32_t memberOffset = memberDesc.Offset;

							const auto type = Utility::GetShaderUniformTypeFromD3D12(memberType);
						
							inOutData.constantsBuffer.AddMember("", type, memberSize, memberOffset);
						}
					}
				}
				else
				{
					auto& buffer = inOutData.uniformBuffers[space][binding];
					buffer.usageStages = buffer.usageStages | stage;
					buffer.usageCount++;
					buffer.size = size;
				}
			}
			else if (shaderInputBindDesc.Type == D3D_SIT_STRUCTURED || shaderInputBindDesc.Type == D3D_SIT_BYTEADDRESS || 
					 shaderInputBindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED || shaderInputBindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
			{
				auto& buffer = inOutData.storageBuffers[space][binding];
				buffer.usageStages = buffer.usageStages | stage;
				buffer.usageCount++;
				buffer.size = 0;
				buffer.isWrite = (shaderInputBindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED || shaderInputBindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS);

				const bool firstEntry = !inOutData.storageBuffers[space].contains(binding);

				if (firstEntry)
				{
					if (shaderInputBindDesc.BindCount == 0)
					{
						buffer.arraySize = -1;
					}
					else
					{
						buffer.arraySize = shaderInputBindDesc.BindCount;
					}
				}

				registerType = buffer.isWrite ? ShaderRegisterType::UnorderedAccess : ShaderRegisterType::Texture;
			}
			else if (shaderInputBindDesc.Type == D3D_SIT_UAV_RWTYPED && (shaderInputBindDesc.Dimension >= 2 && shaderInputBindDesc.Dimension <= 10)) // This is all the texutre types
			{
				auto& shaderImage = inOutData.storageImages[space][binding];
				shaderImage.usageStages = shaderImage.usageStages | stage;
				shaderImage.usageCount++;

				registerType = ShaderRegisterType::UnorderedAccess;

				const bool firstEntry = !inOutData.storageImages[space].contains(binding);

				if (firstEntry)
				{
					if (shaderInputBindDesc.BindCount == 0)
					{
						shaderImage.arraySize = -1;
					}
					else
					{
						shaderImage.arraySize = shaderInputBindDesc.BindCount;
					}
				}
			}
			else if (shaderInputBindDesc.Type == D3D_SIT_TEXTURE)
			{
				auto& shaderImage = inOutData.images[space][binding];
				shaderImage.usageStages = shaderImage.usageStages | stage;
				shaderImage.usageCount++;

				registerType = ShaderRegisterType::Texture;

				const bool firstEntry = !inOutData.images[space].contains(binding);

				if (firstEntry)
				{
					if (shaderInputBindDesc.BindCount == 0)
					{
						shaderImage.arraySize = -1;
					}
					else
					{
						shaderImage.arraySize = shaderInputBindDesc.BindCount;
					}
				}
			}
			else if (shaderInputBindDesc.Type == D3D_SIT_SAMPLER)
			{
				auto& shaderSampler = inOutData.samplers[space][binding];
				shaderSampler.usageStages = shaderSampler.usageStages | stage;
				shaderSampler.usageCount++;

				registerType = ShaderRegisterType::Sampler;
			}

			TryAddShaderBinding(name, space, binding, registerType, inOutData);
		}
	}

	bool D3D12ShaderCompiler::TryAddShaderBinding(const std::string& name, uint32_t set, uint32_t binding, ShaderRegisterType registerType, CompilationResultData& outData)
	{
		if (outData.bindings.contains(name))
		{
			return false;
		}

		outData.bindings[name] = { set, binding, registerType };
		return true;
	}
}
