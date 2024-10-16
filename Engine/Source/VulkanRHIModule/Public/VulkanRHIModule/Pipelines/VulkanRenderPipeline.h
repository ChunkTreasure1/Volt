#pragma once

#include "VulkanRHIModule/Core.h"

#include <RHIModule/Pipelines/RenderPipeline.h>

struct VkPipelineLayout_T;
struct VkPipeline_T;

namespace Volt::RHI
{
	class VulkanRenderPipeline final : public RenderPipeline
	{
	public:
		VulkanRenderPipeline(const RenderPipelineCreateInfo& createInfo);
		~VulkanRenderPipeline() override;
		
		void Invalidate() override;
		RefPtr<Shader> GetShader() const override;

		inline VkPipelineLayout_T* GetPipelineLayout() const { return m_pipelineLayout; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();

		RenderPipelineCreateInfo m_createInfo{};

		VkPipelineLayout_T* m_pipelineLayout = nullptr;
		VkPipeline_T* m_pipeline = nullptr;
	};
}
