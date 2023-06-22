#pragma once

#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/VulkanBarrier.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	class Shader;
	class ShaderStorageBufferSet;
	class UniformBufferSet;
	class Image2D;
	class Image3D;

	enum class ImageAccess
	{ 
		Read,
		Write
	};

	class ComputePipeline
	{
	public:
		ComputePipeline(Ref<Shader> computeShader, uint32_t count, bool useGlobalDescriptors);
		ComputePipeline(Ref<Shader> computeShader, uint32_t count, bool useGlobalDescriptors, const ShaderDataBuffer& specializationConstants);
		~ComputePipeline();

		void Bind(VkCommandBuffer commandBuffer);
		void Dispatch(VkCommandBuffer commandBuffer, uint32_t groupX, uint32_t groupY, uint32_t groupZ, uint32_t index = 0);
		void Clear(uint32_t index = 0);
		void Execute(uint32_t groupX, uint32_t groupY, uint32_t groupZ, const void* pushConstantData = nullptr, const size_t pushConstantSize = 0, uint32_t index = 0);
		void InsertBarriers(VkCommandBuffer commandBuffer, uint32_t index = 0);
		void PushConstants(VkCommandBuffer commandBuffer, const void* data, const size_t size);

		void Invalidate();
		void ClearAllResources();

		void SetUniformBuffer(Ref<UniformBufferSet> uniformBufferSet, uint32_t set, uint32_t binding);
		void SetStorageBuffer(Ref<ShaderStorageBufferSet> shaderStorageBufferSet, uint32_t set, uint32_t binding);

		void SetImage(Ref<Image2D> image, uint32_t set, uint32_t binding, ImageAccess imageAccess);
		void SetImage(Ref<Image2D> image, uint32_t set, uint32_t binding, uint32_t mip, ImageAccess imageAccess);
		void SetImage(Ref<Image2D> image, uint32_t set, uint32_t binding, uint32_t mip, uint32_t arrayElement, ImageAccess imageAccess);

		void SetImage(Ref<Image3D> image, uint32_t set, uint32_t binding, ImageAccess imageAccess);
		void SetImage(Ref<Image3D> image, uint32_t set, uint32_t binding, uint32_t mip, ImageAccess imageAccess);

		void InsertImageBarrier(uint32_t set, uint32_t binding, const ImageBarrierInfo& barrierInfo);
		void InsertUniformBufferBarrier(uint32_t set, uint32_t binding, const BufferBarrierInfo& barrierInfo);
		void InsertStorageBufferBarrier(uint32_t set, uint32_t binding, const BufferBarrierInfo& barrierInfo);
		void InsertStorageBufferBarrier(Ref<ShaderStorageBufferSet> shaderStorageBufferSet, uint32_t set, uint32_t binding, const BufferBarrierInfo& barrierInfo);
		void InsertExecutionBarrier(const ExecutionBarrierInfo& barrierInfo);

		void BindDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, uint32_t set) const;
		bool HasDescriptorSet(uint32_t set) const;

		inline const Ref<Shader> GetShader() const { return myShader; }

		inline ShaderDataBuffer& GetSpecializationConstantBuffer() { return mySpecializationConstantsBuffer; }

		static Ref<ComputePipeline> Create(Ref<Shader> computeShader, uint32_t count = 1, bool useGlobalDescriptors = false);
		static Ref<ComputePipeline> Create(Ref<Shader> computeShader, uint32_t count, bool useGlobalDescriptors, const ShaderDataBuffer& specializationConstants);

	private:
		void Release();
		void SetupFromShader();
		void CreateDescriptorPools();
		void AllocateDescriptorSets(uint32_t index);
		void UpdateDescriptorSets(VkCommandBuffer commandBuffer, uint32_t index);

		void AllocateNonGlobalDescriptorSets(uint32_t index);
		void ResetAfterInvalidate();

		struct Image2DResourceData
		{
			Ref<Image2D> image;
			VkImageLayout imageLayout;
			ImageAccess access;
		};

		struct Image3DResourceData
		{
			Ref<Image3D> image;
			VkImageLayout imageLayout;
			ImageAccess access;
		};

		std::map<uint32_t, std::map<uint32_t, Image2DResourceData>> myImage2DResources;
		std::map<uint32_t, std::map<uint32_t, Image3DResourceData>> myImage3DResources;

		std::map<uint32_t, std::map<uint32_t, Ref<UniformBufferSet>>> myUniformBufferResources;
		std::map<uint32_t, std::map<uint32_t, Ref<ShaderStorageBufferSet>>> myStorageBufferResources;

		std::vector<ShaderResources> myShaderResources;

		std::vector<uint32_t> myWriteDescriptorSetMapping;
		std::vector<std::vector<VkWriteDescriptorSet>> myWriteDescriptors;
		std::map<uint32_t, std::map<uint32_t, uint32_t>> mySetBindingWriteDescriptorMap;

		Ref<Shader> myShader;
		uint32_t myCount;
		bool myUseGlobalDescriptors = false;
		bool myIsPermutation = false;

		VkPipelineLayout myPipelineLayout = nullptr;
		VkPipelineCache myPipelineCache = nullptr;
		VkPipeline myPipeline = nullptr;

		std::vector<uint32_t> myNonGlobalDescriptorSets;

		std::vector<std::vector<VkDescriptorSet>> myDescriptorSets;
		std::vector<VkDescriptorPool> myDescriptorPools;

		std::vector<std::vector<VkImageMemoryBarrier2>> myImageBarriers;
		std::vector<std::vector<VkBufferMemoryBarrier2>> myBufferBarriers;
		std::vector<std::vector<VkMemoryBarrier2>> myExecutionBarriers;

		std::map<uint32_t, std::map<uint32_t, uint32_t>> myImageBarrierIndexMap;
		std::map<uint32_t, std::map<uint32_t, uint32_t>> myBufferBarrierIndexMap;

		ShaderDataBuffer mySpecializationConstantsBuffer;
	};
}
