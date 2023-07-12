#pragma once

#include <VoltRHI/Graphics/GraphicsContext.h>

#include <cstdint>

const char* VKResultToString(int32_t result);

#define VT_VK_CHECK(x) if (x != VK_SUCCESS) { GraphicsContext::Log(Severity::Error, std::format("Vulkan Error: {0}", VKResultToString(x))); VT_RHI_DEBUGBREAK(); }
