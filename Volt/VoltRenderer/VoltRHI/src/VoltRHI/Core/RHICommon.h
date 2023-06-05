#pragma once

namespace Volt
{
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
		Intel
	};

	// --- structures --- \\

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
	};

	struct GraphicsContextCreateInfo
	{
		GraphicsAPI graphicsApi;
		PhysicalDeviceCreateInfo physicalDeviceInfo;
		GraphicsDeviceCreateInfo graphicsDeviceInfo;
	};

}
