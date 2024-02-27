#pragma once

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Core/RHICommon.h>

struct VkShaderModule_T;
struct VkDescriptorSetLayout_T;

namespace Volt::RHI
{
	class VulkanShader final : public Shader
	{
	public:
		struct SourceData
		{
			std::filesystem::path filepath;
			std::string source;
		};

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

		const bool CompileOrGetBinary(bool forceCompile);
		void LoadAndCreateShaders(const std::unordered_map<ShaderStage, std::vector<uint32_t>>& shaderData);
		void ReflectAllStages(const std::unordered_map<ShaderStage, std::vector<uint32_t>>& shaderData);
		void ReflectStage(ShaderStage stage, const std::vector<uint32_t>& data);

		void CreateDescriptorSetLayouts();
		void CalculateDescriptorPoolSizes();

		const bool TryAddShaderBinding(const std::string& name, uint32_t set, uint32_t binding);

		std::unordered_map<ShaderStage, SourceData> m_shaderSources;
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_shaderData;
		std::unordered_map<ShaderStage, PipelineStageInfo> m_pipelineStageInfo;

		std::unordered_map<ShaderStage, TypeCount> m_perStageUBOCount;
		std::unordered_map<ShaderStage, TypeCount> m_perStageSSBOCount;
		std::unordered_map<ShaderStage, TypeCount> m_perStageStorageImageCount;
		std::unordered_map<ShaderStage, TypeCount> m_perStageImageCount;
		std::unordered_map<ShaderStage, TypeCount> m_perStageSamplerCount;

		std::vector<VkDescriptorSetLayout_T*> m_descriptorSetLayouts;
		std::vector<VkDescriptorSetLayout_T*> m_nullPaddedDescriptorSetLayouts;

		std::vector<std::pair<uint32_t, uint32_t>> m_descriptorPoolSizes{}; // Descriptor type -> count

		ShaderSpecification m_specification;
		ShaderResources m_resources;
	};
}
