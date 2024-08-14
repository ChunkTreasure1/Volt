#include "rhipch.h"
#include "Shader.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<Shader> Shader::Create(const ShaderSpecification& createInfo)
	{
		return RHIProxy::GetInstance().CreateShader(createInfo);
	}
}
