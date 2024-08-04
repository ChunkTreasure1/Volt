#include "rhipch.h"
#include "Swapchain.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<Swapchain> Swapchain::Create(GLFWwindow* window)
	{
		return RHIProxy::GetInstance().CreateSwapchain(window);
	}
}
