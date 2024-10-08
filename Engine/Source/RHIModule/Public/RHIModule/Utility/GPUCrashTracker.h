#pragma once

#include "RHIModule/Core/RHICommon.h"

namespace Volt::RHI
{
	class VTRHI_API GPUCrashTracker
	{
	public:
		GPUCrashTracker() = default;
		~GPUCrashTracker();

		void Initialize(GraphicsAPI api);
		void Shutdown();

	private:
		void OnCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);

		void WriteGPUCrashDumpToFile(const void* pGPUCrashDump, const uint32_t gpuCrashDumpSize);

		// Callbacks
		static void GpuCrashDumpCallback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData);

		bool m_initialized = false;
		mutable std::mutex m_mutex;
	};
}
