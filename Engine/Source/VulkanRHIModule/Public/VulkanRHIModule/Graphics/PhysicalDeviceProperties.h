#pragma once

#include "RHIModule/Core/RHICommon.h"

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

	struct PhysicalDeviceLimits
	{
		uint32_t maxImageDimension1D;
		uint32_t maxImageDimension2D;
		uint32_t maxImageDimension3D;
		uint32_t maxImageDimensionCube;
		uint32_t maxImageArrayLayers;
		uint32_t maxTexelBufferElements;
		uint32_t maxUniformBufferRange;
		uint32_t maxStorageBufferRange;
		uint32_t maxPushConstantsSize;
		uint32_t maxMemoryAllocationCount;
		uint32_t maxSamplerAllocationCount;
		uint64_t bufferImageGranularity;
		uint64_t sparseAddressSpaceSize;
		uint32_t maxBoundDescriptorSets;
		uint32_t maxPerStageDescriptorSamplers;
		uint32_t maxPerStageDescriptorUniformBuffers;
		uint32_t maxPerStageDescriptorStorageBuffers;
		uint32_t maxPerStageDescriptorSampledImages;
		uint32_t maxPerStageDescriptorStorageImages;
		uint32_t maxPerStageDescriptorInputAttachments;
		uint32_t maxPerStageResources;
		uint32_t maxDescriptorSetSamplers;
		uint32_t maxDescriptorSetUniformBuffers;
		uint32_t maxDescriptorSetUniformBuffersDynamic;
		uint32_t maxDescriptorSetStorageBuffers;
		uint32_t maxDescriptorSetStorageBuffersDynamic;
		uint32_t maxDescriptorSetSampledImages;
		uint32_t maxDescriptorSetStorageImages;
		uint32_t maxDescriptorSetInputAttachments;
		uint32_t maxVertexInputAttributes;
		uint32_t maxVertexInputBindings;
		uint32_t maxVertexInputAttributeOffset;
		uint32_t maxVertexInputBindingStride;
		uint32_t maxVertexOutputComponents;
		uint32_t maxTessellationGenerationLevel;
		uint32_t maxTessellationPatchSize;
		uint32_t maxTessellationControlPerVertexInputComponents;
		uint32_t maxTessellationControlPerVertexOutputComponents;
		uint32_t maxTessellationControlPerPatchOutputComponents;
		uint32_t maxTessellationControlTotalOutputComponents;
		uint32_t maxTessellationEvaluationInputComponents;
		uint32_t maxTessellationEvaluationOutputComponents;
		uint32_t maxGeometryShaderInvocations;
		uint32_t maxGeometryInputComponents;
		uint32_t maxGeometryOutputComponents;
		uint32_t maxGeometryOutputVertices;
		uint32_t maxGeometryTotalOutputComponents;
		uint32_t maxFragmentInputComponents;
		uint32_t maxFragmentOutputAttachments;
		uint32_t maxFragmentDualSrcAttachments;
		uint32_t maxFragmentCombinedOutputResources;
		uint32_t maxComputeSharedMemorySize;
		uint32_t maxComputeWorkGroupCount[3];
		uint32_t maxComputeWorkGroupInvocations;
		uint32_t maxComputeWorkGroupSize[3];
		uint32_t subPixelPrecisionBits;
		uint32_t subTexelPrecisionBits;
		uint32_t mipmapPrecisionBits;
		uint32_t maxDrawIndexedIndexValue;
		uint32_t maxDrawIndirectCount;
		float maxSamplerLodBias;
		float maxSamplerAnisotropy;
		uint32_t maxViewports;
		uint32_t maxViewportDimensions[2];
		float viewportBoundsRange[2];
		uint32_t viewportSubPixelBits;
		size_t minMemoryMapAlignment;
		uint64_t minTexelBufferOffsetAlignment;
		uint64_t minUniformBufferOffsetAlignment;
		uint64_t minStorageBufferOffsetAlignment;
		int32_t minTexelOffset;
		uint32_t maxTexelOffset;
		int32_t minTexelGatherOffset;
		uint32_t maxTexelGatherOffset;
		float minInterpolationOffset;
		float maxInterpolationOffset;
		uint32_t subPixelInterpolationOffsetBits;
		uint32_t maxFramebufferWidth;
		uint32_t maxFramebufferHeight;
		uint32_t maxFramebufferLayers;
		uint32_t framebufferColorSampleCounts;
		uint32_t framebufferDepthSampleCounts;
		uint32_t framebufferStencilSampleCounts;
		uint32_t framebufferNoAttachmentsSampleCounts;
		uint32_t maxColorAttachments;
		uint32_t sampledImageColorSampleCounts;
		uint32_t sampledImageIntegerSampleCounts;
		uint32_t sampledImageDepthSampleCounts;
		uint32_t sampledImageStencilSampleCounts;
		uint32_t storageImageSampleCounts;
		uint32_t maxSampleMaskWords;
		bool timestampComputeAndGraphics;
		float timestampPeriod;
		uint32_t maxClipDistances;
		uint32_t maxCullDistances;
		uint32_t maxCombinedClipAndCullDistances;
		uint32_t discreteQueuePriorities;
		float pointSizeRange[2];
		float lineWidthRange[2];
		float pointSizeGranularity;
		float lineWidthGranularity;
		bool strictLines;
		bool standardSampleLocations;
		uint64_t optimalBufferCopyOffsetAlignment;
		uint64_t optimalBufferCopyRowPitchAlignment;
		uint64_t nonCoherentAtomSize;
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

		Vector<MemoryType> memoryTypes;
		Vector<MemoryHeap> memoryHeaps;
	};

	struct PhysicalDeviceProperties
	{
		DeviceVendor vendor;
		std::string_view deviceName;

		PhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties;
		PhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties;
		
		PhysicalDeviceMemoryProperties memoryProperties;
		PhysicalDeviceLimits limits;
	};
}
