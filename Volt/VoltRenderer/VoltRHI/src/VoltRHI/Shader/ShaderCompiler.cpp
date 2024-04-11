#include "rhipch.h"
#include "ShaderCompiler.h"

#include "VoltRHI/RHIProxy.h"

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
		return RHIProxy::GetInstance().CreateShaderCompiler(createInfo);
	}
}
