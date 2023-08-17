#pragma once

namespace Volt
{
	class RendererNew
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Flush();

		static const uint32_t GetFramesInFlight();

		static void DestroyResource(std::function<void()>&& function);

	private:
		RendererNew() = delete;
	};
}
