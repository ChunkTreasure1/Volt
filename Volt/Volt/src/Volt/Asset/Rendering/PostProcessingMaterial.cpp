#include "vtpch.h"
#include "PostProcessingMaterial.h"

#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/CommandBuffer.h"

#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/Texture2D.h"

namespace Volt
{
	PostProcessingMaterial::PostProcessingMaterial(Ref<Shader> shader)
		: myShader(shader)
	{
		myPipeline = ComputePipeline::Create(myShader, Renderer::GetFramesInFlightCount(), true);
		Invalidate();

		Renderer::AddShaderDependency(myShader, this);
	}

	PostProcessingMaterial::~PostProcessingMaterial()
	{
		if (myShader)
		{
			Renderer::RemoveShaderDependency(myShader, this);
		}
	}

	void PostProcessingMaterial::Invalidate()
	{
		const auto oldMaterialData = myMaterialData;

		myMaterialData = myShader->CreateShaderBuffer();

		for (const auto& [varName, data] : myMaterialData.GetMembers())
		{
			if (oldMaterialData.HasMember(varName))
			{
				const auto& oldMember = oldMaterialData.GetMember(varName);
				if (oldMember.type == myMaterialData.GetMember(varName).type)
				{
					myMaterialData.SetValue(varName, oldMaterialData.GetValueRaw(varName, oldMember.size), oldMember.size);
				}
			}
		}

		const auto& seperateImages = myShader->GetResources().separateImages;
		const auto oldTextureInfo = myTextures;
		myTextures.clear();

		if (seperateImages.contains(Sets::OTHER))
		{
			for (const auto& [binding, image] : seperateImages.at(Sets::OTHER))
			{
				if (oldTextureInfo.contains(binding))
				{
					myTextures[binding] = TextureInfo{ oldTextureInfo.at(binding).texture, image.name };
				}
				else
				{
					myTextures[binding] = TextureInfo{ Renderer::GetDefaultData().whiteTexture, image.name };
				}
			}
		}
	}

	void PostProcessingMaterial::SetShader(Ref<Shader> shader)
	{
		Renderer::RemoveShaderDependency(myShader, this);
		myShader = shader;

		myPipeline = ComputePipeline::Create(myShader, Renderer::GetFramesInFlightCount(), true);
		Invalidate();

		Renderer::AddShaderDependency(myShader, this);
	}

	void PostProcessingMaterial::Render(Ref<CommandBuffer> commandBuffer, Ref<Image2D> outputImage)
	{
		const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

		myPipeline->Clear(currentIndex);
		myPipeline->SetImage(outputImage, Sets::OTHER, 0, ImageAccess::Write);

		for (const auto& [binding, textureInfo] : myTextures)
		{
			myPipeline->SetImage(textureInfo.texture->GetImage(), Sets::OTHER, binding, ImageAccess::Read);
		}

		myPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

		constexpr uint32_t threadCount = 8;

		const uint32_t dispatchX = std::max(1u, (outputImage->GetWidth() / threadCount) + 1);
		const uint32_t dispatchY = std::max(1u, (outputImage->GetHeight() / threadCount) + 1);

		if (myMaterialData.GetSize() > 0)
		{
			myPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), myMaterialData.GetData(), myMaterialData.GetSize());
		}

		Renderer::DispatchComputePipeline(commandBuffer, myPipeline, dispatchX, dispatchY, 1);
	}

	const std::string& PostProcessingMaterial::GetName() const
	{
		return myPipeline->GetShader()->GetName();
	}
}
