#include "dxpch.h"
#include "D3D12ShaderCompiler.h"

#include <dxc/dxcapi.h>

#include "VoltRHI/Shader/ShaderUtility.h"
#include "VoltRHI/Shader/ShaderPreProcessor.h"

#include "VoltD3D12/Shader/D3D12Shader.h"
#include <d3d12shader.h>

namespace Volt::RHI
{
	D3D12ShaderCompiler::D3D12ShaderCompiler(const ShaderCompilerCreateInfo& createInfo) : m_info(createInfo)
	{
		DxcCreateInstance(CLSID_DxcUtils, VT_D3D12_ID(m_hlslUtils));
		DxcCreateInstance(CLSID_DxcCompiler, VT_D3D12_ID(m_hlslCompiler));
	}

	D3D12ShaderCompiler::~D3D12ShaderCompiler()
	{
	}

	void* D3D12ShaderCompiler::GetHandleImpl() const
	{
		return nullptr;
	}

	ShaderCompiler::CompilationResult D3D12ShaderCompiler::TryCompileImpl(const Specification& specification, Shader& shader)
	{
		auto d3d12shader = reinterpret_cast<D3D12Shader*>(&shader);

		for (auto& path : d3d12shader->m_sourceFiles)
		{
			auto result = CompileStage(path, specification, *d3d12shader);
			if (result != CompilationResult::Success)
			{
				return result;
			}
		}
		return CompilationResult::Success;
	}

	void D3D12ShaderCompiler::AddMacroImpl(const std::string& macroName)
	{
	}

	void D3D12ShaderCompiler::RemoveMacroImpl(std::string_view macroName)
	{
	}
	ShaderCompiler::CompilationResult D3D12ShaderCompiler::CompileStage(const std::filesystem::path& path, const Specification& specification, D3D12Shader& shader)
	{
		auto stageflag = Utility::GetShaderStageFromFilename(path.string());


		std::wstring wEntryPoint(specification.entryPoint.begin(), specification.entryPoint.end());

		std::vector<LPCWSTR> arguments;
		arguments.emplace_back(L"-E");
		arguments.emplace_back(wEntryPoint.c_str());


		Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
		// Get the whole file to string.
		std::ifstream fin(path, std::ios::binary | std::ios::ate);
		fin.seekg(0, std::ios::end);
		std::string file;
		file.resize(fin.tellg());
		fin.seekg(0, std::ios::beg);
		fin.read(file.data(), file.size());


		PreProcessorData ppData = {};
		ppData.entryPoint = specification.entryPoint;
		ppData.shaderStage = stageflag;
		ppData.shaderSource = file;

		PreProcessorResult ppResult = {};

		if (!ShaderPreProcessor::PreProcessShaderSource(ppData, ppResult))
		{
			return ShaderCompiler::CompilationResult::PreprocessFailed;
		}

		file = ppResult.preProcessedResult;
		
		m_hlslUtils->CreateBlob(file.data(), static_cast<uint32_t>(file.size()), CP_UTF8, sourceBlob.GetAddressOf());
		auto target = Utility::HLSLShaderProfile(stageflag);
		arguments.push_back(L"-T");
		arguments.push_back(target);


		switch (specification.optimizationLevel)
		{
			case OptimizationLevel::Release:
				arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
				break;
			case OptimizationLevel::Dist:
				arguments.push_back(DXC_ARG_SKIP_VALIDATION);
				break;
			case OptimizationLevel::Disable:
				break;
			default:
				break;
		}

		switch (m_info.flags)
		{
			case ShaderCompilerFlags::None:
				break;
			case ShaderCompilerFlags::WarningsAsErrors:
				arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
				break;
		}

		arguments.push_back(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR);
		/*if (stageFlag != ShaderStageFlag::Pixel)
		{
			arguments.push_back(L"-fvk-invert-y");
		}*/

		DxcBuffer sourceBuffer{};
		sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
		sourceBuffer.Size = sourceBlob->GetBufferSize();
		sourceBuffer.Encoding = 0;

		Microsoft::WRL::ComPtr<IDxcResult> compileResult;
		auto hr = m_hlslCompiler->Compile(&sourceBuffer, arguments.data(), (uint32_t)arguments.size(), nullptr, IID_PPV_ARGS(compileResult.GetAddressOf()));
		if (FAILED(hr))
		{
			Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrors;
			IDxcBlobWide* wide = nullptr;
			hr = compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.GetAddressOf()), &wide);
			if (pErrors && pErrors->GetStringLength() > 0)
			{
				GraphicsContext::LogTagged(Severity::Error, "[D3D12 Shader]", (char*)pErrors->GetBufferPointer());
			}

			return ShaderCompiler::CompilationResult::Failure;
		}
		
		IDxcBlob* blob = nullptr;
		compileResult->GetResult(&blob);

		shader.m_blobMap[stageflag] = blob;


		if (stageflag == ShaderStage::Vertex)
		{
			shader.m_resources.vertexLayout = ppResult.vertexLayout;
		}


		if(stageflag == ShaderStage::Pixel)
		{
			shader.m_resources.outputFormats = ppResult.outputFormats;
		}

		return ShaderCompiler::CompilationResult::Success;
	}
}
