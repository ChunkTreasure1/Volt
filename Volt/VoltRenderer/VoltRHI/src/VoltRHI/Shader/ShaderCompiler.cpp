#include "rhipch.h"
#include "ShaderCompiler.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Shader/VulkanShaderCompiler.h>

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
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;
			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanShaderCompiler>(createInfo); break;
		}

		return nullptr;
	}
}
