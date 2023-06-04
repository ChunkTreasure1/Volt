#include "FinalLayer.h"

#include <Volt/Core/Application.h>

#include <Volt/Events/KeyEvent.h>
#include <Volt/Input/KeyCodes.h>

FinalLayer::FinalLayer(Ref<Volt::SceneRenderer>& aSceneRenderer)
	: mySceneRenderer(aSceneRenderer)
{}

void FinalLayer::OnAttach()
{
	//myCopyToScreenShader = Volt::ShaderRegistry::Get("CopyTextureToTarget");
}

void FinalLayer::OnDetach()
{}

void FinalLayer::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(FinalLayer::OnRenderEvent));

	dispatcher.Dispatch<Volt::KeyPressedEvent>([&](Volt::KeyPressedEvent& e)
		{
			//if (e.GetKeyCode() == VT_KEY_F6)
			//{
			//	const auto& allBuffers = mySceneRenderer->GetAllFramebuffers();

			//	uint32_t currentPassTargetCount = 0;
			//	for (const auto& att : allBuffers.at(myPassIndex).second->GetSpecification().attachments)
			//	{
			//		if (!Volt::Utility::IsDepthFormat(att.format))
			//		{
			//			currentPassTargetCount++;
			//		}
			//	}

			//	if (myDebugTargets)
			//	{
			//		myTargetIndex++;
			//	}

			//	myDebugTargets = true;
			//	if (myTargetIndex == currentPassTargetCount - 1)
			//	{
			//		myTargetIndex = 0;
			//		myPassIndex = 0;
			//		myDebugTargets = false;
			//	}

			//}
			return false;
		});
}

bool FinalLayer::OnRenderEvent(Volt::AppRenderEvent& e)
{
	//Volt::Application::Get().GetWindow().GetSwapchain().Bind();

	//Volt::RenderCommand::SetContext(Volt::Context::Immidiate);
	//
	////myCopyToScreenShader->Bind();

	//if (!myDebugTargets)
	//{
	//	Volt::RenderCommand::BindSRVsToStage({ mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0) }, Volt::ShaderStage::Pixel, 0);
	//}
	//else
	//{
	//	Volt::RenderCommand::BindSRVsToStage({ mySceneRenderer->GetAllFramebuffers().at(myPassIndex).second->GetColorAttachment(myTargetIndex) }, Volt::ShaderStage::Pixel, 0);
	//}

	//Volt::RenderCommand::SetTopology(Volt::Topology::TriangleList);
	//Volt::RenderCommand::IndexBuffer_Bind(nullptr);
	//Volt::RenderCommand::VertexBuffer_Bind(nullptr);

	//Volt::RenderCommand::Draw(3, 0);
	//
	////myCopyToScreenShader->Unbind();

	//Volt::RenderCommand::ClearSRVsAtStages(1, Volt::ShaderStage::Pixel, 0);

	return false;
}
