#pragma once

#include "VoltVulkan/Core.h"
#include <VoltRHI/Pipelines/ComputePipeline.h>

struct VkPipeline_T;
struct VkPipelineLayout_T;

namespace Volt::RHI
{
	class VulkanComputePipeline : public ComputePipeline
	{ 
	public:
		VulkanComputePipeline(RefPtr<Shader> shader, bool useGlobalResources);
		~VulkanComputePipeline() override;

		void Invalidate() override;
		RefPtr<Shader> GetShader() const override;

		inline VkPipelineLayout_T* GetPipelineLayout() const { return m_pipelineLayout; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();

		RefPtr<Shader> m_shader;
		bool m_useGlobalResouces = false;

		VkPipeline_T* m_pipeline = nullptr;
		VkPipelineLayout_T* m_pipelineLayout = nullptr;
	};
}
