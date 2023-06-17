#include "vtpch.h"
#include "SubMaterial.h"

#include "Volt/Core/Graphics/GraphicsContext.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/RenderPipeline/RenderPipeline.h"

#include "Volt/Rendering/Shader/ShaderUtility.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/Texture/TextureTable.h"

#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Vertex.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	SubMaterial::SubMaterial()
	{
		Renderer::AddMaterial(this);
	}

	SubMaterial::SubMaterial(const SubMaterial& subMaterial)
	{
		myName = subMaterial.myName;
		myIndex = subMaterial.myIndex;
		myTopology = subMaterial.myTopology;
		myCullMode = subMaterial.myCullMode;
		myTriangleFillMode = subMaterial.myTriangleFillMode;
		myDepthMode = subMaterial.myDepthMode;
		myMaterialData = subMaterial.myMaterialData;

		myPipelineGenerationData = subMaterial.myPipelineGenerationData;
		myMaterialSpecializationData = subMaterial.myMaterialSpecializationData;
		myMaterialFlags = subMaterial.myMaterialFlags;

		InvalidatePipeline(subMaterial.myPipeline->GetSpecification().shader);
		Invalidate();
		GenerateHash();

		myTextures = subMaterial.myTextures;
		Renderer::AddMaterial(this);
		Renderer::AddShaderDependency(myPipeline->GetSpecification().shader, this);
	}

	SubMaterial::SubMaterial(const std::string& aName, uint32_t aIndex, Ref<Shader> shader)
		: myName(aName), myIndex(aIndex)
	{
		InvalidatePipeline(shader);
		Invalidate();

		if (shader->GetName() == "IllumTransparent")
		{
			SetFlag(MaterialFlag::Opaque, false);
			SetFlag(MaterialFlag::SSS, false);
			SetFlag(MaterialFlag::Deferred, false);
			SetFlag(MaterialFlag::Transparent, true);
		}
		else if (shader->GetName() == "IllumSSS")
		{
			SetFlag(MaterialFlag::Opaque, false);
			SetFlag(MaterialFlag::Transparent, false);
			SetFlag(MaterialFlag::Deferred, false);
			SetFlag(MaterialFlag::SSS, true);
		}
		else if (shader->GetName() == "Illum")
		{
			SetFlag(MaterialFlag::Opaque, false);
			SetFlag(MaterialFlag::Transparent, false);
			SetFlag(MaterialFlag::SSS, false);
			SetFlag(MaterialFlag::Deferred, true);
		}
		else if (shader->GetName() == "Decal")
		{
			SetFlag(MaterialFlag::Opaque, false);
			SetFlag(MaterialFlag::Transparent, false);
			SetFlag(MaterialFlag::SSS, false);
			SetFlag(MaterialFlag::Deferred, false);
			SetFlag(MaterialFlag::Decal, true);
		}

		Renderer::AddMaterial(this);
		Renderer::AddShaderDependency(shader, this);
	}

	SubMaterial::SubMaterial(const std::string& aName, uint32_t aIndex, const RenderPipelineSpecification& specification)
	{
		InvalidatePipeline(specification);
		Invalidate();

		Renderer::AddMaterial(this);
		Renderer::AddShaderDependency(specification.shader, this);
	}

	SubMaterial::~SubMaterial()
	{
		Release();

		Renderer::RemoveMaterial(this);
		Renderer::RemoveShaderDependency(myPipeline->GetSpecification().shader, this);
	}

	void SubMaterial::Bind(Ref<CommandBuffer> commandBuffer)
	{
		VT_PROFILE_FUNCTION();

		myPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
		UpdateDescriptorSetsForRendering(commandBuffer);

		// Set texture uint values
		for (const auto& [shaderName, texture] : myTextures)
		{
			if (shaderName == "albedo" || shaderName == "normal" || shaderName == "material")
			{
				continue;
			}

			uint32_t textureIndex = 0;
			if (texture)
			{
				textureIndex = Renderer::GetBindlessData().textureTable->GetBindingFromTexture(texture->GetImage());
			}

			if (myMaterialSpecializationData.HasMember(shaderName))
			{
				myMaterialSpecializationData.SetValue(shaderName, textureIndex);
			}
		}

		if (!myMaterialDescriptorSets.empty())
		{
			myPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myMaterialDescriptorSets.at(commandBuffer->GetCurrentIndex()), Sets::MATERIAL);
		}
	}

	void SubMaterial::PushMaterialData(Ref<CommandBuffer> commandBuffer) const
	{
		VT_PROFILE_FUNCTION();

		if (myMaterialSpecializationData.GetSize() == 0)
		{
			return;
		}

		myPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), myMaterialSpecializationData.GetData(), myMaterialSpecializationData.GetSize());
	}

	void SubMaterial::SetShader(Ref<Shader> shader)
	{
		Renderer::RemoveShaderDependency(myPipeline->GetSpecification().shader, this);
		Renderer::AddShaderDependency(shader, this);

		InvalidatePipeline(shader);
		Invalidate();

		Renderer::UpdateMaterial(this);
	}

	void SubMaterial::SetTexture(const std::string& name, Ref<Texture2D> texture)
	{
		if (auto it = myTextures.find(name); it == myTextures.end())
		{
			VT_CORE_ERROR("No texture exists with name {0} in material {1}!", name, myName.c_str());
			return;
		}

		myTextures[name] = texture;
		Renderer::UpdateMaterial(this);
	}

	void SubMaterial::SetFlag(MaterialFlag flag, bool state)
	{
		if (state)
		{
			myMaterialFlags = myMaterialFlags | flag;
		}
		else
		{
			myMaterialFlags = myMaterialFlags & ~(flag);
		}

		GenerateHash();
		Renderer::UpdateMaterial(this);
	}

	bool SubMaterial::operator==(const SubMaterial& rhs)
	{
		return myHash == rhs.myHash;
	}

	bool SubMaterial::operator!=(const SubMaterial& rhs)
	{
		return myHash != rhs.myHash;
	}

	void SubMaterial::Invalidate()
	{
		const auto originalTextures = myTextures;
		myTextures.clear();

		for (const auto& descriptorPool : myMaterialDescriptorPools)
		{
			vkDestroyDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), descriptorPool, nullptr);
		}

		myMaterialDescriptorPools.clear();
		myMaterialDescriptorSets.clear();

		SetupMaterialFromPipeline();

		for (auto& [binding, texture] : myTextures)
		{
			auto it = originalTextures.find(binding);
			if (it != originalTextures.end())
			{
				texture = it->second;
			}
		}

		GenerateHash();
		Renderer::UpdateMaterial(this);
	}

	void SubMaterial::InvalidatePipeline()
	{
		InvalidatePipeline(myPipeline->GetSpecification().shader);
	}

	void SubMaterial::RecompilePermutation()
	{
		const auto attachments = GetAttachmentsFromMaterialFlags();
		myPipeline->SetAttachments(attachments);

		if (!myPipeline->IsPermutation())
		{
			myPipeline = RenderPipeline::Create(myPipeline->GetSpecification(), myPipelineGenerationData, myPipeline->GetHash());
		}
		else
		{
			myPipeline->Invalidate(myPipelineGenerationData);
		}

		Renderer::UpdateMaterial(this);
	}

	Ref<SubMaterial> SubMaterial::Create(const std::string& aName, uint32_t aIndex, Ref<Shader> aShader)
	{
		return CreateRef<SubMaterial>(aName, aIndex, aShader);
	}

	Ref<SubMaterial> SubMaterial::Create(const std::string& aName, uint32_t aIndex, const RenderPipelineSpecification& specification)
	{
		return CreateRef<SubMaterial>(aName, aIndex, specification);
	}

	Ref<SubMaterial> SubMaterial::Create()
	{
		return CreateRef<SubMaterial>();
	}

	void SubMaterial::Set(uint32_t binding, Ref<Image2D> image)
	{
		myMaterialImages[binding] = image;
		InvalidateDescriptorSets();
	}

	void SubMaterial::SetupMaterialFromPipeline()
	{
		const auto& resources = myPipeline->GetSpecification().shader->GetResources();
		myMaterialWriteDescriptors = resources.materialWriteDescriptors;

		// Transfer material data
		{
			auto oldMaterialData = myMaterialSpecializationData;
			myMaterialSpecializationData = myPipeline->GetSpecification().shader->CreateShaderBuffer();

			const auto& oldMembers = oldMaterialData.GetMembers();

			for (const auto& [name, uniform] : myMaterialSpecializationData.GetMembers())
			{
				if (oldMembers.contains(name) && oldMembers.at(name).size == uniform.size)
				{
					myMaterialSpecializationData.SetValue(name, oldMaterialData.GetValueRaw(name, oldMembers.at(name).size), oldMembers.at(name).size);
				}
			}

			oldMaterialData.Release();
		}

		// Transfer pipeline generation data
		{
			auto oldGenerationData = myPipelineGenerationData;
			myPipelineGenerationData.clear();
			for (const auto& [stage, buffer] : resources.specializationConstantBuffers)
			{
				ShaderStage voltStage = (ShaderStage)stage;

				auto newBuffer = myPipeline->GetSpecification().shader->CreateSpecialiazationConstantsBuffer(voltStage);

				if (oldGenerationData.contains(voltStage))
				{
					const auto& oldMembers = oldGenerationData.at(voltStage).GetMembers();

					for (const auto& [name, uniform] : myPipelineGenerationData.at(voltStage).GetMembers())
					{
						if (oldMembers.contains(name) && oldMembers.at(name).size == uniform.size)
						{
							newBuffer.SetValue(name, oldGenerationData.at(voltStage).GetValueRaw(name, oldMembers.at(name).size), oldMembers.at(name).size);
						}
					}

					oldGenerationData.at(voltStage).Release();
				}

				myPipelineGenerationData[voltStage] = newBuffer;
			}
		}

		// Add default
		{
			myTextures.emplace("albedo", Renderer::GetDefaultData().whiteTexture);
			myTextures.emplace("normal", Renderer::GetDefaultData().whiteTexture);
			myTextures.emplace("material", Renderer::GetDefaultData().whiteTexture);
		}

		for (const auto& [shaderName, editorName] : resources.shaderTextureDefinitions)
		{
			myTextures.emplace(shaderName, Renderer::GetDefaultData().whiteTexture);
		}

		if (!resources.materialWriteDescriptors.empty())
		{
			constexpr VkDescriptorPoolSize poolSizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 },
			};

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
			poolInfo.maxSets = (uint32_t)resources.materialWriteDescriptors.size();
			poolInfo.poolSizeCount = (uint32_t)ARRAYSIZE(poolSizes);
			poolInfo.pPoolSizes = poolSizes;

			const uint32_t framesInFlight = Renderer::GetFramesInFlightCount();

			myMaterialDescriptorPools.resize(framesInFlight);
			myMaterialDescriptorSets.resize(framesInFlight);
			myDirtyDescriptorSets = std::vector<bool>(framesInFlight, false);

			for (uint32_t i = 0; i < framesInFlight; i++)
			{
				VT_VK_CHECK(vkCreateDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), &poolInfo, nullptr, &myMaterialDescriptorPools[i]));
				myMaterialDescriptorSets[i] = myPipeline->GetSpecification().shader->AllocateDescriptorSet(Sets::MATERIAL, myMaterialDescriptorPools[i]);
			}
		}
	}

	void SubMaterial::InvalidateDescriptorSets()
	{
		VT_PROFILE_FUNCTION();

		for (auto&& myDirtyDescriptorSet : myDirtyDescriptorSets)
		{
			myDirtyDescriptorSet = true;
		}
	}

	void SubMaterial::UpdateDescriptorSetsForRendering(Ref<CommandBuffer> commandBuffer)
	{
		VT_PROFILE_FUNCTION();

		if (myMaterialWriteDescriptors.empty())
		{
			return;
		}

		if (myDirtyDescriptorSets.at(commandBuffer->GetCurrentIndex()))
		{
			for (auto& writeDescriptor : myMaterialWriteDescriptors)
			{
				if (!myMaterialImages.contains(writeDescriptor.dstBinding))
				{
					continue;
				}

				const bool isDepth = Utility::IsDepthFormat(myMaterialImages.at(writeDescriptor.dstBinding)->GetFormat());

				writeDescriptor.pImageInfo = &myMaterialImages.at(writeDescriptor.dstBinding)->GetDescriptorInfo(isDepth ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				writeDescriptor.dstSet = myMaterialDescriptorSets.at(commandBuffer->GetCurrentIndex());
			}

			vkUpdateDescriptorSets(GraphicsContext::GetDevice()->GetHandle(), (uint32_t)myMaterialWriteDescriptors.size(), myMaterialWriteDescriptors.data(), 0, nullptr);
			myDirtyDescriptorSets.at(commandBuffer->GetCurrentIndex()) = false;
		}
	}

	void SubMaterial::GenerateHash()
	{
		myHash = 0;
		myHash = Utility::HashCombine(myHash, myPipeline->GetHash());
		myHash = Utility::HashCombine(myHash, std::hash<std::string>()(myName));
		myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()(myIndex));
		myHash = Utility::HashCombine(myHash, std::hash<size_t>()(myTextures.size()));
		myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myMaterialFlags));
		myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myTopology));
		myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myCullMode));
		myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myTriangleFillMode));
		myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myDepthMode));
	}

	void SubMaterial::Release()
	{
		myMaterialSpecializationData.Release();

		for (auto& buffer : myPipelineGenerationData)
		{
			buffer.second.Release();
		}

		for (const auto& descriptorPool : myMaterialDescriptorPools)
		{
			vkDestroyDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), descriptorPool, nullptr);
		}
	}

	void SubMaterial::InvalidatePipeline(Ref<Shader> shader)
	{
		RenderPipelineSpecification spec{};
		spec.shader = shader;
		spec.topology = myTopology;
		spec.cullMode = myCullMode;
		spec.fillMode = myTriangleFillMode;
		spec.depthMode = myDepthMode;
		spec.vertexLayout = Vertex::GetVertexLayout();
		spec.name = myName;
		spec.framebufferAttachments = GetAttachmentsFromMaterialFlags();

		myPipeline = RenderPipeline::Create(spec);
		GenerateHash();
	}

	void SubMaterial::InvalidatePipeline(const RenderPipelineSpecification& specification)
	{
		myPipeline = RenderPipeline::Create(specification);

		myTopology = specification.topology;
		myCullMode = specification.cullMode;
		myTriangleFillMode = specification.fillMode;
		myDepthMode = specification.depthMode;

		GenerateHash();
	}

	const std::vector<FramebufferAttachment> SubMaterial::GetAttachmentsFromMaterialFlags() const
	{
		std::vector<FramebufferAttachment> framebufferAttachments;
		if ((myMaterialFlags & MaterialFlag::Transparent) != MaterialFlag::All)
		{
			framebufferAttachments = RenderPipelineSpecification::GetDefaultTransparentAttachments();
		}
		else if ((myMaterialFlags & MaterialFlag::SSS) != MaterialFlag::All)
		{
			framebufferAttachments = RenderPipelineSpecification::GetDefaultSSSAttachments();
		}
		else if ((myMaterialFlags & MaterialFlag::Decal) != MaterialFlag::All)
		{
			framebufferAttachments = RenderPipelineSpecification::GetDefaultDecalAttachments();
		}
		else
		{
			framebufferAttachments = RenderPipelineSpecification::DefaultForwardAttachments();
		}

		return framebufferAttachments;
	}

	bool operator>(const SubMaterial& lhs, const SubMaterial& rhs)
	{
		return lhs.myHash > rhs.myHash;
	}

	bool operator<(const SubMaterial& lhs, const SubMaterial& rhs)
	{
		return lhs.myHash < rhs.myHash;
	}
}
