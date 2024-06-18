#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Shader/ShaderCompiler.h>
#include <VoltRHI/Core/RHICommon.h>

struct VkShaderModule_T;
struct VkDescriptorSetLayout_T;

namespace Volt::RHI
{
	class VulkanShader final : public Shader
	{
	public:
		struct PipelineStageInfo
		{
			VkShaderModule_T* shaderModule;
		};

		VulkanShader(const ShaderSpecification& createInfo);
		~VulkanShader() override;

		const bool Reload(bool forceCompile) override;
		std::string_view GetName() const override;
		const std::vector<std::filesystem::path>& GetSourceFiles() const override;
		const ShaderResources& GetResources() const override;
		const ShaderResourceBinding& GetResourceBindingFromName(std::string_view name) const override;
		ShaderDataBuffer GetConstantsBuffer() const override;

		inline const std::vector<std::pair<uint32_t, uint32_t>>& GetDescriptorPoolSizes() const { return m_descriptorPoolSizes; }
		inline const std::vector<VkDescriptorSetLayout_T*>& GetDescriptorSetLayouts() const { return m_descriptorSetLayouts; }
		inline const std::vector<VkDescriptorSetLayout_T*>& GetPaddedDescriptorSetLayouts() const { return m_nullPaddedDescriptorSetLayouts; }
		inline const std::unordered_map<ShaderStage, PipelineStageInfo>& GetPipelineStageInfos() const { return m_pipelineStageInfo; }

	protected:
		void* GetHandleImpl() const override;

	private:
		friend class VulkanShaderCompiler;

		struct TypeCount
		{
			uint32_t count = 0;
		};

		void LoadShaderFromFiles();
		void Release();

		ShaderCompiler::CompilationResultData CompileOrGetBinary(bool forceCompile);
		void LoadAndCreateShaders(const std::unordered_map<ShaderStage, std::vector<uint32_t>>& shaderData);

		void CreateDescriptorSetLayouts();
		void CalculateDescriptorPoolSizes(const ShaderCompiler::CompilationResultData& compilationResult);
		void CopyCompilationResults(const ShaderCompiler::CompilationResultData& compilationResult);

		std::unordered_map<ShaderStage, ShaderSourceInfo> m_shaderSources;
		std::unordered_map<ShaderStage, PipelineStageInfo> m_pipelineStageInfo;

		std::vector<VkDescriptorSetLayout_T*> m_descriptorSetLayouts;
		std::vector<VkDescriptorSetLayout_T*> m_nullPaddedDescriptorSetLayouts;

		std::vector<std::pair<uint32_t, uint32_t>> m_descriptorPoolSizes{}; // Descriptor type -> count

		ShaderSpecification m_specification;
		ShaderResources m_resources;
	};
}
