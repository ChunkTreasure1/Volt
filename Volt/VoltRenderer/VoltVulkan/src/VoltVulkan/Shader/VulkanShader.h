#pragma once

#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Core/RHICommon.h>

struct VkShaderModule_T;
struct VkDescriptorSetLayout_T;

namespace Volt::RHI
{
	class VulkanShader : public Shader
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

		VulkanShader(std::string_view name, const std::vector<std::filesystem::path>& sourceFiles, bool forceCompile);
		~VulkanShader() override;

		const bool Reload(bool forceCompile) override;
		std::string_view GetName() const override;
		const ShaderResources& GetResources() const override;
		const std::vector<std::filesystem::path>& GetSourceFiles() const override;
		ShaderDataBuffer GetConstantsBuffer() const override;

	protected:
		void* GetHandleImpl() override;

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

		std::unordered_map<ShaderStage, SourceData> m_shaderSources;
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_shaderData;
		std::unordered_map<ShaderStage, PipelineStageInfo> m_pipelineStageInfo;

		std::unordered_map<ShaderStage, TypeCount> m_perStageUBOCount;
		std::unordered_map<ShaderStage, TypeCount> m_perStageSSBOCount;
		std::unordered_map<ShaderStage, TypeCount> m_perStageStorageImageCount;
		std::unordered_map<ShaderStage, TypeCount> m_perStageImageCount;
		std::unordered_map<ShaderStage, TypeCount> m_perStageSamplerCount;

		std::vector<VkDescriptorSetLayout_T*> m_descriptorSetLayouts;

		std::string_view m_name;
		std::vector<std::filesystem::path> m_sourceFiles;
		ShaderResources m_resources;
	};
}
