#include "rhipch.h"
#include "RHIModule/Shader/ShaderCompiler.h"

#include "RHIModule/RHIProxy.h"

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

	ShaderCompiler::CompilationResultData ShaderCompiler::TryCompile(const Specification& specification)
	{
		return s_instance->TryCompileImpl(specification);
	}

	void ShaderCompiler::AddMacro(const std::string& macroName)
	{
		s_instance->AddMacroImpl(macroName);
	}

	void ShaderCompiler::RemoveMacro(std::string_view macroName)
	{
		s_instance->RemoveMacroImpl(macroName);
	}

	RefPtr<ShaderCompiler> ShaderCompiler::Create(const ShaderCompilerCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateShaderCompiler(createInfo);
	}
}
