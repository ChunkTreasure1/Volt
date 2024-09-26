#include "rhipch.h"

#include "RHIModule/Utility/GPUCrashTracker.h"

#ifdef VT_ENABLE_NV_AFTERMATH

#include "RHIModule/Utility/NsightAftermathHelpers.h"

#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_Defines.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>

#endif

namespace Volt::RHI
{
#ifdef VT_ENABLE_NV_AFTERMATH
	inline const uint32_t GetAPIFlag(const GraphicsAPI api)
	{
		switch (api)
		{
			case GraphicsAPI::Vulkan: return GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan;
			case GraphicsAPI::D3D12: return GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX;

			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;
		}

		return GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_None;
	}
#endif

	GPUCrashTracker::~GPUCrashTracker()
	{
		if (m_initialized)
		{
			Shutdown();
		}
	}

	void GPUCrashTracker::Initialize(GraphicsAPI api)
	{
		m_initialized = true;
	
#ifdef VT_ENABLE_NV_AFTERMATH
		AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_EnableGpuCrashDumps(
			GFSDK_Aftermath_Version_API,
			GetAPIFlag(api),
			GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks,
			GpuCrashDumpCallback,
			nullptr,
			nullptr,
			nullptr,
			this));
#endif
	}

	void GPUCrashTracker::Shutdown()
	{
		m_initialized = false;

#ifdef VT_ENABLE_NV_AFTERMATH
		GFSDK_Aftermath_DisableGpuCrashDumps();
#endif

	}

	void GPUCrashTracker::OnCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
	{
		std::scoped_lock lock{ m_mutex };
		WriteGPUCrashDumpToFile(pGpuCrashDump, gpuCrashDumpSize);
	}

	void GPUCrashTracker::WriteGPUCrashDumpToFile(const void* pGPUCrashDump, const uint32_t gpuCrashDumpSize)
	{
#ifdef VT_ENABLE_NV_AFTERMATH

		GFSDK_Aftermath_GpuCrashDump_Decoder decoder{};
		AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_CreateDecoder(GFSDK_Aftermath_Version_API, pGPUCrashDump, gpuCrashDumpSize, &decoder));

		GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo{};
		AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo));

		uint32_t applicationNameLength = 0;
		AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize(decoder, GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, &applicationNameLength));
	
		//Vector<char> applicationName(applicationNameLength, '\0');

		//AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetDescription(decoder, GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, 
		//	uint32_t(applicationName.size()), applicationName.data()));
	
		const std::string crashDumpFileName = std::string("GPUCrashDump") + ".nv-gpudmp";
		std::ofstream dumpFile(crashDumpFileName, std::ios::out | std::ios::binary);
		if (dumpFile)
		{
			dumpFile.write((const char*)pGPUCrashDump, gpuCrashDumpSize);
			dumpFile.close();
		}
#endif
	}

	void GPUCrashTracker::GpuCrashDumpCallback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData)
	{
		GPUCrashTracker* gpuCrashTracker = reinterpret_cast<GPUCrashTracker*>(pUserData);
		gpuCrashTracker->OnCrashDump(pGpuCrashDump, gpuCrashDumpSize);
	}
}
