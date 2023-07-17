#pragma once
#include "VoltRHI/Core/Core.h"

namespace Volt
{
	class PhysicalGraphicsDevice;
	class GraphicsDevice;

	enum class QueueType
	{
		Graphics,
		Compute,
		TransferCopy
	};

	enum class GraphicsAPI
	{
		Vulkan,
		D3D12,
		MoltenVk,
		Mock,
	};

	enum class DeviceVendor
	{
		AMD,
		NVIDIA,
		Intel,

		Unknown
	};

	enum class Severity
	{
		Trace,
		Info,
		Warning,
		Error,
	};

	// --- structures --- \\

	struct LogHookInfo
	{
		std::function<void(Severity, std::string_view)> logCallback;
		bool enabled = false;
	};

	struct PhysicalDeviceCreateInfo
	{
	};

	struct DeviceCapabilities
	{
		DeviceVendor deviceVendor;
		std::string_view gpuName;
	};

	struct GraphicsDeviceCreateInfo
	{
		Ref<PhysicalGraphicsDevice> physicalDevice;
	};

	struct GraphicsContextCreateInfo
	{
		GraphicsAPI graphicsApi;
		PhysicalDeviceCreateInfo physicalDeviceInfo;
		GraphicsDeviceCreateInfo graphicsDeviceInfo;
		LogHookInfo loghookInfo;
	};

	struct DeviceQueueCreateInfo
	{
		GraphicsDevice* graphicsDevice;
		QueueType queueType;
	};
}
