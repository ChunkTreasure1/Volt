#include "vkpch.h"
#include "VulkanCommandBuffer.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanSwapchain.h"
#include "VoltVulkan/Common/VulkanHelpers.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/DeviceQueue.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>

#include <VoltRHI/Images/ImageView.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanCommandBuffer::VulkanCommandBuffer(const uint32_t count, QueueType queueType, bool swapchainTarget)
		: CommandBuffer(queueType), m_commandBufferCount(count), m_isSwapchainTarget(swapchainTarget)
	{
		m_currentCommandBufferIndex = m_commandBufferCount - 1; // This makes sure that we start at command buffer index 0 for clarity

		Invalidate();
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Release();
	}

	void VulkanCommandBuffer::Begin()
	{
		m_currentCommandBufferIndex = (m_currentCommandBufferIndex + 1) % m_commandBufferCount;

		auto device = GraphicsContext::GetDevice();
		const uint32_t index = GetCurrentCommandBufferIndex();
		const auto& currentCommandBuffer = m_commandBuffers.at(index);

		if (!m_isSwapchainTarget)
		{
			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence, VK_TRUE, UINT64_MAX));
			VT_VK_CHECK(vkResetFences(device->GetHandle<VkDevice>(), 1, &currentCommandBuffer.fence));
			VT_VK_CHECK(vkResetCommandPool(device->GetHandle<VkDevice>(), currentCommandBuffer.commandPool, 0));
		}

		// Begin command buffer
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VT_VK_CHECK(vkBeginCommandBuffer(currentCommandBuffer.commandBuffer, &beginInfo));
		}
	}

	void VulkanCommandBuffer::End()
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		VT_VK_CHECK(vkEndCommandBuffer(m_commandBuffers.at(index).commandBuffer));
	}

	void VulkanCommandBuffer::Execute()
	{
		if (!m_isSwapchainTarget)
		{
			auto device = GraphicsContext::GetDevice();
			device->GetDeviceQueue(m_queueType)->Execute({ As<VulkanCommandBuffer>() });
		}
	}

	void VulkanCommandBuffer::ExecuteAndWait()
	{
		if (!m_isSwapchainTarget)
		{
			auto device = GraphicsContext::GetDevice();
			device->GetDeviceQueue(m_queueType)->Execute({ As<VulkanCommandBuffer>() });

			const uint32_t index = GetCurrentCommandBufferIndex();
			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &m_commandBuffers.at(index).fence, VK_TRUE, UINT64_MAX));
		}
	}

	void VulkanCommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdDraw(m_commandBuffers.at(index).commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdDrawIndexed(m_commandBuffers.at(index).commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::SetViewports(const std::vector<Viewport>& viewports)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdSetViewport(m_commandBuffers.at(index).commandBuffer, 0, static_cast<uint32_t>(viewports.size()), reinterpret_cast<const VkViewport*>(viewports.data()));
	}

	void VulkanCommandBuffer::SetScissors(const std::vector<Rect2D>& scissors)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdSetScissor(m_commandBuffers.at(index).commandBuffer, 0, static_cast<uint32_t>(scissors.size()), reinterpret_cast<const VkRect2D*>(scissors.data()));
	}

	void VulkanCommandBuffer::BindPipeline(Ref<RenderPipeline> pipeline)
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdBindPipeline(m_commandBuffers.at(index).commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetHandle<VkPipeline>());
	}

	void VulkanCommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
	{
		std::vector<VkRenderingAttachmentInfo> colorAttachmentInfo{};
		VkRenderingAttachmentInfo depthAttachmentInfo{};

		for (const auto& colorAtt : renderingInfo.colorAttachments)
		{
			auto& newInfo = colorAttachmentInfo.emplace_back();
			newInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			newInfo.imageView = colorAtt.view.lock()->GetHandle<VkImageView>();
			newInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			newInfo.loadOp = Utility::VoltToVulkanLoadOp(colorAtt.clearMode);
			newInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			newInfo.clearValue = { colorAtt.clearColor[0], colorAtt.clearColor[1], colorAtt.clearColor[2], colorAtt.clearColor[3] };
		}

		if (!renderingInfo.depthAttachmentInfo.view.expired())
		{
			depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachmentInfo.imageView = renderingInfo.depthAttachmentInfo.view.lock()->GetHandle<VkImageView>();
			depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			depthAttachmentInfo.loadOp = Utility::VoltToVulkanLoadOp(renderingInfo.depthAttachmentInfo.clearMode);
			depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachmentInfo.clearValue.depthStencil = { renderingInfo.depthAttachmentInfo.clearColor[0], static_cast<uint32_t>(renderingInfo.depthAttachmentInfo.clearColor[1]) };
		}

		VkRenderingInfo vkRenderingInfo{};
		vkRenderingInfo.renderArea = { renderingInfo.renderArea.offset.x, renderingInfo.renderArea.offset.y, renderingInfo.renderArea.extent.width, renderingInfo.renderArea.extent.height };
		vkRenderingInfo.layerCount = renderingInfo.layerCount;
		vkRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfo.size());
		vkRenderingInfo.pColorAttachments = colorAttachmentInfo.data();
		vkRenderingInfo.pStencilAttachment = nullptr;

		if (!renderingInfo.depthAttachmentInfo.view.expired())
		{
			vkRenderingInfo.pDepthAttachment = &depthAttachmentInfo;
		}
		else
		{
			vkRenderingInfo.pDepthAttachment = nullptr;
		}

		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdBeginRendering(m_commandBuffers.at(index).commandBuffer, &vkRenderingInfo);
	}

	void VulkanCommandBuffer::EndRendering()
	{
		const uint32_t index = GetCurrentCommandBufferIndex();
		vkCmdEndRendering(m_commandBuffers.at(index).commandBuffer);
	}

	VkFence_T* VulkanCommandBuffer::GetCurrentFence() const
	{
		return m_commandBuffers.at(m_currentCommandBufferIndex).fence;
	}

	void* VulkanCommandBuffer::GetHandleImpl()
	{
		return m_commandBuffers.at(m_currentCommandBufferIndex).commandBuffer;
	}

	void VulkanCommandBuffer::Invalidate()
	{
		auto physicalDevice = GraphicsContext::GetPhysicalDevice()->As<VulkanPhysicalGraphicsDevice>();
		auto device = GraphicsContext::GetDevice();

		const auto& queueFamilies = physicalDevice->GetQueueFamilies();

		if (m_isSwapchainTarget)
		{
			auto& swapchain = VulkanSwapchain::Get();
			m_commandBufferCount = VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
			m_commandBuffers.resize(m_commandBufferCount);

			for (uint32_t i = 0; i < m_commandBufferCount; i++)
			{
				m_commandBuffers[i].commandBuffer = swapchain.GetCommandBuffer(i);
				m_commandBuffers[i].commandPool = swapchain.GetCommandPool(i);
			}

			return;
		}
		else
		{
			m_commandBuffers.resize(m_commandBufferCount);

			for (uint32_t i = 0; i < m_commandBufferCount; i++)
			{
				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

				uint32_t queueFamilyIndex = 0;

				switch (m_queueType)
				{
					case QueueType::Graphics:
						queueFamilyIndex = queueFamilies.graphicsFamilyQueueIndex;
						break;
					case QueueType::Compute:
						queueFamilyIndex = queueFamilies.computeFamilyQueueIndex;
						break;
					case QueueType::TransferCopy:
						queueFamilyIndex = queueFamilies.transferFamilyQueueIndex;
						break;
					default:
						assert(false);
						break;
				}

				poolInfo.queueFamilyIndex = queueFamilyIndex;
				poolInfo.flags = 0;

				VT_VK_CHECK(vkCreateCommandPool(device->GetHandle<VkDevice>(), &poolInfo, nullptr, &m_commandBuffers[i].commandPool));
			}

			for (uint32_t i = 0; i < m_commandBufferCount; i++)
			{
				VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = m_commandBuffers[i].commandPool;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = 1;

				VT_VK_CHECK(vkAllocateCommandBuffers(device->GetHandle<VkDevice>(), &allocInfo, &m_commandBuffers[i].commandBuffer));
			}

			for (uint32_t i = 0; i < m_commandBufferCount; i++)
			{
				VkFenceCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

				VT_VK_CHECK(vkCreateFence(device->GetHandle<VkDevice>(), &info, nullptr, &m_commandBuffers[i].fence));
			}
		}
	}

	void VulkanCommandBuffer::Release()
	{
		auto device = GraphicsContext::GetDevice();
		if (!m_isSwapchainTarget)
		{
			std::vector<VkFence> fences{};
			for (const auto& cmdBuffer : m_commandBuffers)
			{
				fences.emplace_back(cmdBuffer.fence);
			}

			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));

			for (auto& cmdBuffer : m_commandBuffers)
			{
				vkDestroyFence(device->GetHandle<VkDevice>(), cmdBuffer.fence, nullptr);
				vkDestroyCommandPool(device->GetHandle<VkDevice>(), cmdBuffer.commandPool, nullptr);
			}
		}
		else
		{
			std::vector<VkFence> fences{};
			for (uint32_t i = 0; i < VulkanSwapchain::MAX_FRAMES_IN_FLIGHT; i++)
			{
				fences.push_back(VulkanSwapchain::Get().GetFence(i));
			}

			VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));
		}

		m_commandBuffers.clear();
	}

	const uint32_t VulkanCommandBuffer::GetCurrentCommandBufferIndex() const
	{
		return m_currentCommandBufferIndex;
	}
}
