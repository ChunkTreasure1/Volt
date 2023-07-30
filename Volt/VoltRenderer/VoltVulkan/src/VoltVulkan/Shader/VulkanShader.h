#pragma once

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Core/RHICommon.h>

namespace Volt::RHI
{
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(std::string_view name, const std::vector<std::filesystem::path>& sourceFiles, bool forceCompile);
		~VulkanShader() override;

		const bool Reload(bool forceCompile) override;

	protected:
		void* GetHandleImpl() override;

	private:
		void LoadShaderFromFiles();
		void Release();

		const bool CompileOrGetBinary(std::unordered_map<ShaderStage, std::vector<uint32_t>>& outShaderData, bool forceCompile);
		void LoadAndCreateShaders(const std::unordered_map<ShaderStage, std::vector<uint32_t>>& shaderData);
		void ReflectAllStages(const std::unordered_map<ShaderStage, std::vector<uint32_t>>& shaderData);
		void ReflectStage(ShaderStage stage, const std::vector<uint32_t>& data);

		std::unordered_map<ShaderStage, std::string> m_shaderSources;
	};
}
