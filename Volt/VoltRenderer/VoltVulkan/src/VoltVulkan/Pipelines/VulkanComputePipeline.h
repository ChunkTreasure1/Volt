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

		inline VkPipelineLayout_T* GetPipelineLayout() const { return m_pipelineLayout; }
		inline Ref<Shader> GetShader() const { return m_shader; }

	protected:
		void* GetHandleImpl() override;

	private:
		void Release();

		Ref<Shader> m_shader;

		VkPipeline_T* m_pipeline = nullptr;
		VkPipelineLayout_T* m_pipelineLayout = nullptr;
	};
}
