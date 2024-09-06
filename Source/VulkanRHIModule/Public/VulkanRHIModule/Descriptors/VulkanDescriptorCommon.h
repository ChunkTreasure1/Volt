#pragma once

#include <cstdint>

struct VkDescriptorSet_T;
struct VkBufferView_T;

struct VkSampler_T;
struct VkImageView_T;
struct VkBuffer_T;

struct VkDescriptorImageInfo;
struct VkDescriptorBufferInfo;

namespace Volt::RHI
{
	struct DescriptorWrite
	{
		uint32_t sType = 0;
		const void* pNext = nullptr;
		VkDescriptorSet_T* dstSet = nullptr;
		uint32_t dstBinding = 0;
		uint32_t dstArrayElement = 0;
		uint32_t descriptorCount = 0;
		uint32_t descriptorType = 0;
		const VkDescriptorImageInfo* pImageInfo;
		const VkDescriptorBufferInfo* pBufferInfo;
		const VkBufferView_T* pTexelBufferView;
	};

	struct DescriptorImageInfo
	{
		VkSampler_T* sampler = nullptr;
		VkImageView_T* imageView = nullptr;
		uint32_t imageLayout = 0;
	};

	struct DescriptorBufferInfo
	{
		VkBuffer_T* buffer = nullptr;
		uint64_t offset = 0;
		uint64_t range = 0;
	};
}
