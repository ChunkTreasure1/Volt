#pragma once

#include "Volt/RenderingNew/SamplerLibrary.h"

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

		static Ref<RHI::SamplerState> GetSampler(SamplerType samplerType);

	private:
		RendererNew() = delete;
	};
}
