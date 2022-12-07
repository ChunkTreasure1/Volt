#include "FinalLayer.h"

#include <Volt/Core/Application.h>
#include <Volt/Core/Graphics/Swapchain.h>

#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Framebuffer.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>

#include <Volt/Events/KeyEvent.h>
#include <Volt/Input/KeyCodes.h>

FinalLayer::FinalLayer(Ref<Volt::SceneRenderer>& aSceneRenderer)
	: mySceneRenderer(aSceneRenderer)
{}

void FinalLayer::OnAttach()
{
	myCopyToScreenShader = Volt::ShaderRegistry::Get("CopyTextureToTarget");
}

void FinalLayer::OnDetach()
{}

void FinalLayer::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(FinalLayer::OnRenderEvent));

	dispatcher.Dispatch<Volt::KeyPressedEvent>([&](Volt::KeyPressedEvent& e)
		{
			if (e.GetKeyCode() == VT_KEY_F6)
			{
				const auto& allBuffers = mySceneRenderer->GetAllFramebuffers();

				uint32_t currentPassTargetCount = 0;
				for (const auto& att : allBuffers.at(myPassIndex).second->GetSpecification().attachments)
				{
					if (!Volt::Utility::IsDepthFormat(att.format))
					{
						currentPassTargetCount++;
					}
				}

				if (myDebugTargets)
				{
					myTargetIndex++;
				}

				myDebugTargets = true;
				if (myTargetIndex == currentPassTargetCount - 1)
				{
					myTargetIndex = 0;
					myPassIndex = 0;
					myDebugTargets = false;
				}

			}
			return false;
		});
}

bool FinalLayer::OnRenderEvent(Volt::AppRenderEvent& e)
{
	Volt::Application::Get().GetWindow().GetSwapchain().Bind();

	auto context = Volt::GraphicsContext::GetContext();

	if (!myDebugTargets)
	{
		context->PSSetShaderResources(0, 1, mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0)->GetSRV().GetAddressOf());
	}
	else
	{
		context->PSSetShaderResources(0, 1, mySceneRenderer->GetAllFramebuffers().at(myPassIndex).second->GetColorAttachment(myTargetIndex)->GetSRV().GetAddressOf());
	}

	Volt::Renderer::DrawFullscreenTriangleWithShader(myCopyToScreenShader);

	ID3D11ShaderResourceView* srv = nullptr;
	context->PSSetShaderResources(0, 1, &srv);

	return false;
}
