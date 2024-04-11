#include "rhipch.h"
#include "ShaderCompiler.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Shader/VulkanShaderCompiler.h>
#include <VoltD3D12/Shader/D3D12ShaderCompiler.h>

namespace Volt::RHI
{
	ShaderCompiler::ShaderCompiler()
	{
		s_instance = this;
	}

	ShaderCompiler::~ShaderCompiler()
	{
		s_instance = nullptr;
	}

	ShaderCompiler::CompilationResult ShaderCompiler::TryCompile(const Specification& specification, Shader& shader)
	{
		return s_instance->TryCompileImpl(specification, shader);
	}

	void ShaderCompiler::AddMacro(const std::string& macroName)
	{
		s_instance->AddMacroImpl(macroName);
	}

	void ShaderCompiler::RemoveMacro(std::string_view macroName)
	{
		s_instance->RemoveMacroImpl(macroName);
	}

	Ref<ShaderCompiler> ShaderCompiler::Create(const ShaderCompilerCreateInfo& createInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			//case GraphicsAPI::D3D12: return CreateRef<D3D12ShaderCompiler>(createInfo); break;
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;
			case GraphicsAPI::Vulkan: return CreateVulkanShaderCompiler(createInfo); break;
		}

		return nullptr;
	}
}
