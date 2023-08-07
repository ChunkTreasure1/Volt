#pragma once

#include <VoltRHI/Pipelines/RenderPipeline.h>

struct VkPipelineLayout_T;
struct VkPipeline_T;

namespace Volt::RHI
{
	class VulkanRenderPipeline : public RenderPipeline
	{
	public:
		VulkanRenderPipeline(const RenderPipelineCreateInfo& createInfo);
		~VulkanRenderPipeline() override;
		
		void Invalidate() override;

	protected:
		void* GetHandleImpl() override;

	private:
		void Release();

		RenderPipelineCreateInfo m_createInfo{};

		VkPipelineLayout_T* m_pipelineLayout = nullptr;
		VkPipeline_T* m_pipeline = nullptr;
	};
}
