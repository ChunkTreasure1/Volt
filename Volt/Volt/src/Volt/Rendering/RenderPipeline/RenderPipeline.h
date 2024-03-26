#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/Buffer/BufferLayout.h"
#include "Volt/Rendering/VulkanFramebuffer.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace Volt
{
	class Shader;
	struct VulkanRenderPass;

	enum class Topology : uint32_t
	{
		TriangleList = 0,
		LineList,
		TriangleStrip,
		PatchList,
		PointList
	};

	enum class CullMode : uint32_t
	{
		Front = 0,
		Back,
		FrontAndBack,
		None
	};

	enum class FillMode : uint32_t
	{
		Solid = 0,
		Wireframe
	};

	enum class DepthMode : uint32_t
	{
		Read = 0,
		Write,
		ReadWrite,
		None
	};

	struct RenderPipelineStatistics
	{
		uint64_t inputAssemblyVertices = 0;
		uint64_t inputAssemblyPrimitives = 0;
		uint64_t vertexShaderInvocations = 0;
		uint64_t clippingInvocations = 0;
		uint64_t clippingPrimitives = 0;
		uint64_t fragmentShaderInvocations = 0;
		uint64_t computeShaderInvocations = 0;
	};

	struct RenderPipelineSpecification
	{
		Ref<Shader> shader;
		Weak<VulkanRenderPass> renderPass;
		
		std::vector<FramebufferAttachment> framebufferAttachments = // Used if render pass is null
		{
			{ ImageFormat::RGBA16F, { 1.f }, TextureBlend::Alpha },
			{ ImageFormat::DEPTH32F }
		}; 

		Topology topology = Topology::TriangleList;
		CullMode cullMode = CullMode::Back;
		FillMode fillMode = FillMode::Solid;
		DepthMode depthMode = DepthMode::ReadWrite;
		CompareOperator depthCompareOperator = CompareOperator::GreaterEqual;

		float lineWidth = 1.f;
		uint32_t tessellationControlPoints = 4;

		BufferLayout vertexLayout;
		BufferLayout instanceLayout;

		std::string name;

		inline static std::vector<FramebufferAttachment> DefaultForwardAttachments()
		{
			std::vector<FramebufferAttachment> attachments = // Used if render pass is null
			{
				{ ImageFormat::RGBA16F, { 1.f }, TextureBlend::Alpha },
				{ ImageFormat::DEPTH32F }
			};

			return attachments;
		}

		inline static std::vector<FramebufferAttachment> GetDefaultTransparentAttachments()
		{
			std::vector<FramebufferAttachment> attachments = // Used if render pass is null
			{
				{ ImageFormat::RGBA16F, { 0.f }, TextureBlend::Add },
				{ ImageFormat::R8U, { 1.f }, TextureBlend::OneMinusSrcColor },
				{ ImageFormat::DEPTH32F }
			};

			return attachments;
		}

		inline static std::vector<FramebufferAttachment> GetDefaultSSSAttachments()
		{
			std::vector<FramebufferAttachment> attachments = // Used if render pass is null
			{
				{ ImageFormat::RGBA16F, { 0.f }, TextureBlend::Alpha },
				{ ImageFormat::RGBA16F, { 0.f }, TextureBlend::Alpha },
				{ ImageFormat::DEPTH32F }
			};

			return attachments;
		}

		inline static std::vector<FramebufferAttachment> GetDefaultDecalAttachments()
		{
			std::vector<FramebufferAttachment> attachments = // Used if render pass is null
			{
				{ ImageFormat::RGBA, { 0.f } },
				{ ImageFormat::RGBA16F, { 0.f } },
				{ ImageFormat::RGBA16F, { 0.f } }
			};

			return attachments;
		}
	};

	class RenderPipeline : public Asset
	{
	public:
		RenderPipeline() = default;
		RenderPipeline(const RenderPipelineSpecification& specification);
		RenderPipeline(const RenderPipelineSpecification& specification, const std::map<ShaderStage, ShaderDataBuffer>& specializationConstants, size_t permutationHash);
		~RenderPipeline();

		void Invalidate(const std::map<ShaderStage, ShaderDataBuffer>& specializationConstants = {});
		void Recreate(const RenderPipelineSpecification& specification);

		void Bind(VkCommandBuffer commandBuffer) const;
		void BindDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, uint32_t set) const;
		void PushConstants(VkCommandBuffer commandBuffer, const void* data, const size_t size) const;

		bool HasDescriptorSet(uint32_t set) const;

		void SetAttachments(const std::vector<FramebufferAttachment>& attachments);

		inline const RenderPipelineSpecification& GetSpecification() const { return mySpecification; }
		inline const std::string& GetName() const { return mySpecification.name; }
		inline const size_t GetHash() const { return myHash; }
		inline const bool IsPermutation() const { return myIsPermutation; }

		const bool IsPermutationOf(Ref<RenderPipeline> renderPipeline) const;

		static Ref<RenderPipeline> Create(const RenderPipelineSpecification& specification);
		static Ref<RenderPipeline> Create(const RenderPipelineSpecification& specification, const std::map<ShaderStage, ShaderDataBuffer>& specializationConstants, size_t permutationHash);

		static AssetType GetStaticType() { return AssetType::RenderPipeline; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }

	private:
		friend class RenderPipelineImporter;

		void Release();
		void CreateVertexLayout();
		void GenerateHash();

		RenderPipelineSpecification mySpecification;
		std::map<ShaderStage, ShaderDataBuffer> mySpecializationConstantsBuffers;

		size_t myHash = 0;
		size_t myPermutationHash = 0;

		bool myIsPermutation = false;

		std::vector<VkVertexInputBindingDescription> myVertexBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> myVertexAttributeDescriptions;

		VkPipelineLayout myPipelineLayout = nullptr;
		VkPipeline myPipeline = nullptr;
	};
}
