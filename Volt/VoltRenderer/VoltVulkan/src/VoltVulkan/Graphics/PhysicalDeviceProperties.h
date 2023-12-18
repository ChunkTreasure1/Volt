#pragma once

#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	struct PhysicalDeviceDescriptorBufferPropertiesEXT
	{
		bool enabled = false;

		bool combinedImageSamplerDescriptorSingleArray;
		bool bufferlessPushDescriptors;
		bool allowSamplerImageViewPostSubmitCreation;
		uint64_t descriptorBufferOffsetAlignment;
		uint32_t maxDescriptorBufferBindings;
		uint32_t maxResourceDescriptorBufferBindings;
		uint32_t maxSamplerDescriptorBufferBindings;
		uint32_t maxEmbeddedImmutableSamplerBindings;
		uint32_t maxEmbeddedImmutableSamplers;
		size_t bufferCaptureReplayDescriptorDataSize;
		size_t imageCaptureReplayDescriptorDataSize;
		size_t imageViewCaptureReplayDescriptorDataSize;
		size_t samplerCaptureReplayDescriptorDataSize;
		size_t accelerationStructureCaptureReplayDescriptorDataSize;
		size_t samplerDescriptorSize;
		size_t combinedImageSamplerDescriptorSize;
		size_t sampledImageDescriptorSize;
		size_t storageImageDescriptorSize;
		size_t uniformTexelBufferDescriptorSize;
		size_t robustUniformTexelBufferDescriptorSize;
		size_t storageTexelBufferDescriptorSize;
		size_t robustStorageTexelBufferDescriptorSize;
		size_t uniformBufferDescriptorSize;
		size_t robustUniformBufferDescriptorSize;
		size_t storageBufferDescriptorSize;
		size_t robustStorageBufferDescriptorSize;
		size_t inputAttachmentDescriptorSize;
		size_t accelerationStructureDescriptorSize;
		uint64_t maxSamplerDescriptorBufferRange;
		uint64_t maxResourceDescriptorBufferRange;
		uint64_t samplerDescriptorBufferAddressSpaceSize;
		uint64_t resourceDescriptorBufferAddressSpaceSize;
		uint64_t descriptorBufferAddressSpaceSize;
	};

	struct PhysicalDeviceMeshShaderPropertiesEXT
	{
		bool enabled = false;

		uint32_t maxTaskWorkGroupTotalCount;
		uint32_t maxTaskWorkGroupCount[3];
		uint32_t maxTaskWorkGroupInvocations;
		uint32_t maxTaskWorkGroupSize[3];
		uint32_t maxTaskPayloadSize;
		uint32_t maxTaskSharedMemorySize;
		uint32_t maxTaskPayloadAndSharedMemorySize;
		uint32_t maxMeshWorkGroupTotalCount;
		uint32_t maxMeshWorkGroupCount[3];
		uint32_t maxMeshWorkGroupInvocations;
		uint32_t maxMeshWorkGroupSize[3];
		uint32_t maxMeshSharedMemorySize;
		uint32_t maxMeshPayloadAndSharedMemorySize;
		uint32_t maxMeshOutputMemorySize;
		uint32_t maxMeshPayloadAndOutputMemorySize;
		uint32_t maxMeshOutputComponents;
		uint32_t maxMeshOutputVertices;
		uint32_t maxMeshOutputPrimitives;
		uint32_t maxMeshOutputLayers;
		uint32_t maxMeshMultiviewViewCount;
		uint32_t meshOutputPerVertexGranularity;
		uint32_t meshOutputPerPrimitiveGranularity;
		uint32_t maxPreferredTaskWorkGroupInvocations;
		uint32_t maxPreferredMeshWorkGroupInvocations;
		bool prefersLocalInvocationVertexOutput;
		bool prefersLocalInvocationPrimitiveOutput;
		bool prefersCompactVertexOutput;
		bool prefersCompactPrimitiveOutput;
	};

	struct PhysicalDeviceMemoryProperties
	{
		struct MemoryType
		{
			uint32_t propertyFlags;
			uint32_t heapIndex;
			uint32_t index;
		};

		struct MemoryHeap
		{
			uint64_t size;
			uint32_t flags;
		};

		std::vector<MemoryType> memoryTypes;
		std::vector<MemoryHeap> memoryHeaps;
	};

	struct PhysicalDeviceProperties
	{
		DeviceVendor vendor;
		std::string_view deviceName;

		bool hasTimestampSupport = false;
		float timestampPeriod = 0.f;

		float maxAnisotropyLevel = 1.f;
	
		PhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties;
		PhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties;
		PhysicalDeviceMemoryProperties memoryProperties;
	};
}
