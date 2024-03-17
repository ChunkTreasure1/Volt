#pragma once

#include <VoltRHI/Pipelines/ComputePipeline.h>

struct VkPipeline_T;
struct VkPipelineLayout_T;

namespace Volt::RHI
{
	class VulkanComputePipeline : public ComputePipeline
	{ 
	public:
		VulkanComputePipeline(Ref<Shader> shader);
		~VulkanComputePipeline() override;

		void Invalidate() override;
		Ref<Shader> GetShader() const override;

		inline VkPipelineLayout_T* GetPipelineLayout() const { return m_pipelineLayout; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();

		Ref<Shader> m_shader;

		VkPipeline_T* m_pipeline = nullptr;
		VkPipelineLayout_T* m_pipelineLayout = nullptr;
	};
}
