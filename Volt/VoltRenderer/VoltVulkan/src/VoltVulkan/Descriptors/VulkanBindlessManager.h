#pragma once

namespace Volt::RHI
{
	class VulkanBindlessManager
	{
	public:
		static void CreateGlobalDescriptorLayout();
		static void DestroyGlobalDescriptorLayout();

	private:

		VulkanBindlessManager() = delete;
	};
}
