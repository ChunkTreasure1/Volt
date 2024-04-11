#include "rhipch.h"
#include "Swapchain.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<Swapchain> Swapchain::Create(GLFWwindow* window)
	{
		return RHIProxy::GetInstance().CreateSwapchain(window);
	}
}
