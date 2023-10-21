#pragma once

#include "Volt/RenderingNew/RendererStructs.h"

namespace Volt
{
	namespace RHI
	{
		class SamplerState;
	}

	class Mesh;
	class RendererNew
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Flush();
		static const uint32_t GetFramesInFlight();
		static void DestroyResource(std::function<void()>&& function);

		static SamplersData GetSamplersData();

	private:
		static void RegisterSamplers();
		static void UnregisterSamplers();

		RendererNew() = delete;
	};
}
