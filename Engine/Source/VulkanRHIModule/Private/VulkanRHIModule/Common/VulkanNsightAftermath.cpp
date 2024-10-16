#include "vkpch.h"
#include "VulkanNsightAftermath.h"

#ifdef VT_ENABLE_NV_AFTERMATH

#include "VulkanRHIModule/Common/VulkanCommon.h"

#include <RHIModule/Utility/NsightAftermathHelpers.h>
#include <RHIModule/RHIProxy.h>

#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_Defines.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	void CheckWaitReturnValue(uint32_t resultValue)
	{
		VkResult vkResult = static_cast<VkResult>(resultValue);

		if (vkResult == VK_ERROR_DEVICE_LOST)
		{
			//VT_LOGC(Error, LogTemp, "Graphics device lost! Generating Nsight Aftermath report and closing application!");

			auto tdrTerminationTimeout = std::chrono::seconds(3);
			auto tStart = std::chrono::steady_clock::now();
			auto tElapsed = std::chrono::milliseconds::zero();

			GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

			while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed && status != GFSDK_Aftermath_CrashDump_Status_Finished && tElapsed < tdrTerminationTimeout)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

				auto tEnd = std::chrono::steady_clock::now();
				tElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart);
			}

			if (status != GFSDK_Aftermath_CrashDump_Status_Finished)
			{
				//VT_LOGC(Error, "[Aftermath]", "Unexpected crash dump status: {0}", static_cast<uint32_t>(status));
			}

			RHIProxy::GetInstance().RequestApplicationClose();

			// #TODO_Ivar: Add ability to trigger application shutdown
		}

		if (resultValue != VK_SUCCESS) 
		{ 
			//VT_LOGC(Error, LogTemp, "Vulkan Error: {0}", VKResultToString(resultValue));
			//VT_DEBUGBREAK(); 
		}
	}
}

#endif
