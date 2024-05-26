#pragma once

#ifdef VT_ENABLE_NV_AFTERMATH

#include "VoltVulkan/Common/VulkanNsightAftermath.h"

#endif

#include <VoltRHI/Graphics/GraphicsContext.h>

#include <cstdint>

const char* VKResultToString(int32_t result);

#ifndef VT_ENABLE_NV_AFTERMATH

#define VT_VK_CHECK(x) if (x != VK_SUCCESS) { RHILog::Log(Severity::Error, std::format("Vulkan Error: {0}", VKResultToString(x))); VT_RHI_DEBUGBREAK(); }

#else

#define VT_VK_CHECK(x) Volt::RHI::CheckWaitReturnValue(x)

#endif

namespace VulkanDefaults
{
	constexpr uint32_t ShaderBRegisterOffset = 0;
	constexpr uint32_t ShaderTRegisterOffset = 100;
	constexpr uint32_t ShaderURegisterOffset = 200;

	constexpr uint32_t STORAGE_BUFFER_BINDLESS_TABLE_SIZE = 8192;
	constexpr uint32_t STORAGE_IMAGE_BINDLESS_TABLE_SIZE = 8192;
	constexpr uint32_t IMAGE_BINDLESS_TABLE_SIZE = 8192;
};
