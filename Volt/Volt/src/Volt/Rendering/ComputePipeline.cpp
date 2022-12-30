#include "vtpch.h"
#include "ComputePipeline.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Framebuffer.h"
#include "Volt/Rendering/RenderCommand.h"

namespace Volt
{
	ComputePipeline::ComputePipeline(Ref<Shader> aShader)
		: myShader(aShader)
	{}

	ComputePipeline::~ComputePipeline()
	{
		myShader = nullptr;
	}

	void ComputePipeline::Execute(uint32_t aGroupX, uint32_t aGroupY, uint32_t aGroupZ)
	{
		const size_t maxSRVs = gem::max(myImagesT.size(), myTexturesT.size());
		std::vector<Ref<Image2D>> allSRVs{ maxSRVs, nullptr };

		for (size_t i = 0; i < maxSRVs; i++)
		{
			if (i < myImagesT.size() && myImagesT.at(i))
			{
				allSRVs[i] = myImagesT.at(i);
			}

			if (i < myTexturesT.size() && myTexturesT.at(i))
			{
				allSRVs[i] = myTexturesT.at(i)->GetImage();
			}
		}

		if (!allSRVs.empty())
		{
			RenderCommand::BindTexturesToStage(ShaderStage::Compute, allSRVs);
		}

		std::vector<Ref<Image2D>> allUAVs{ myTargetsT.size(), nullptr };
		for (size_t i = 0; i < allUAVs.size(); i++)
		{
			if (myTargetsT[i])
			{
				allUAVs[i] = myTargetsT[i];
			}
		}

		if (!allUAVs.empty())
		{
			RenderCommand::BindComputeResources(allUAVs);
		}

		myShader->Bind();
		RenderCommand::Dispatch(aGroupX, aGroupY, aGroupZ);
	}

	void ComputePipeline::Clear()
	{
		auto context = GraphicsContext::GetImmediateContext();

		const size_t maxSRVs = gem::max(myTexturesT.size(), myImagesT.size());

		RenderCommand::ClearTexturesAtStage(ShaderStage::Compute, 0, maxSRVs);

		if (!myTargetsT.empty())
		{
			RenderCommand::ClearComputeResources(0, myTargetsT.size());
		}
	}

	void ComputePipeline::SetImage(Ref<Image2D> aImage, uint32_t binding)
	{
		if (myImagesT.size() <= (size_t)binding)
		{
			if (myImagesT.size() + 1 == (size_t)binding)
			{
				myImagesT.emplace_back(aImage);
			}
			else
			{
				while (myImagesT.size() + 1 < (size_t)binding)
				{
					myImagesT.emplace_back(nullptr);
				}

				myImagesT.emplace_back(aImage);
			}
		}
		else
		{
			myImagesT[binding] = aImage;
		}
	}

	void ComputePipeline::SetTexture(Ref<Texture2D> aImage, uint32_t aBinding)
	{
		if (myTexturesT.size() <= (size_t)aBinding)
		{
			if (myTexturesT.size() + 1 == (size_t)aBinding)
			{
				myTexturesT.emplace_back(aImage);
			}
			else
			{
				while (myTexturesT.size() + 1 < (size_t)aBinding)
				{
					myTexturesT.emplace_back(nullptr);
				}

				myTexturesT.emplace_back(aImage);
			}
		}
		else
		{
			myTexturesT[aBinding] = aImage;
		}
	}

	void ComputePipeline::SetTarget(Ref<Image2D> aImage, uint32_t aBinding)
	{
		if (myTargetsT.size() <= (size_t)aBinding)
		{
			if (myTargetsT.size() + 1 == (size_t)aBinding)
			{
				myTargetsT.emplace_back(aImage);
			}
			else
			{
				while (myTargetsT.size() + 1 < (size_t)aBinding)
				{
					myTargetsT.emplace_back(nullptr);
				}

				myTargetsT.emplace_back(aImage);
			}
		}
		else
		{
			myTargetsT[aBinding] = aImage;
		}
	}

	Ref<ComputePipeline> ComputePipeline::Create(Ref<Shader> aShader)
	{
		return CreateRef<ComputePipeline>(aShader);
	}
}