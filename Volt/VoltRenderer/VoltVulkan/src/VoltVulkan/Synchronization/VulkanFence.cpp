#include "vkpch.h"
#include "VulkanFence.h"

#include "VoltVulkan/Common/VulkanCommon.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>

#include <VoltRHI/RHIProxy.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	VulkanFence::VulkanFence(const FenceCreateInfo& createInfo)
	{
		VkFenceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = createInfo.createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkCreateFence(device->GetHandle<VkDevice>(), &info, nullptr, &m_fence));
	}

	VulkanFence::~VulkanFence()
	{
		RHIProxy::GetInstance().DestroyResource([fence = m_fence]()
		{
			auto device = GraphicsContext::GetDevice();
			vkDestroyFence(device->GetHandle<VkDevice>(), fence, nullptr);
		});
	}

	void VulkanFence::Reset() const
	{
		auto device = GraphicsContext::GetDevice();
		vkResetFences(device->GetHandle<VkDevice>(), 1, &m_fence);
	}

	FenceStatus VulkanFence::GetStatus() const
	{
		auto device = GraphicsContext::GetDevice();
		VkResult result = vkGetFenceStatus(device->GetHandle<VkDevice>(), m_fence);

		if (result == VK_SUCCESS)
		{
			return FenceStatus::Signaled;
		}
		else if (result == VK_NOT_READY)
		{
			return FenceStatus::Unsignaled;
		}

		return FenceStatus::Error;
	}

	void VulkanFence::WaitUntilSignaled() const
	{
		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &m_fence, VK_TRUE, UINT64_MAX));
	}

	void* VulkanFence::GetHandleImpl() const
	{
		return m_fence;
	}
}
