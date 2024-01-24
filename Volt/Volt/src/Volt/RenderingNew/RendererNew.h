#pragma once

#include "Volt/RenderingNew/RendererStructs.h"
#include "Volt/RenderingNew/Resources/GlobalResource.h"

#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/Core/RHICommon.h>

namespace Volt
{
	namespace RHI
	{
		class SamplerState;
		struct SamplerStateCreateInfo;
	}

	class Texture2D;

	struct DefaultResources
	{
		Ref<Texture2D> whiteTexture;
	};

	class Mesh;
	class RendererNew
	{
	public:
		static void PreInitialize();
		static void Initialize();
		static void Shutdown();

		static void Flush();
		static const uint32_t GetFramesInFlight();
		static void DestroyResource(std::function<void()>&& function);

		static const DefaultResources& GetDefaultResources();

		static void Update();

		template<RHI::TextureFilter min, RHI::TextureFilter mag, RHI::TextureFilter mip, RHI::TextureWrap wrapMode = RHI::TextureWrap::Repeat, RHI::AnisotropyLevel aniso = RHI::AnisotropyLevel::None>
		static Ref<GlobalResource<RHI::SamplerState>> GetSampler()
		{
			RHI::SamplerStateCreateInfo info{};
			info.minFilter = min;
			info.magFilter = mag;
			info.mipFilter = mip;
			info.wrapMode = wrapMode;
			info.anisotropyLevel = aniso;

			return GetSamplerInternal(info);
		}

	private:
		static Ref<GlobalResource<RHI::SamplerState>> GetSamplerInternal(const RHI::SamplerStateCreateInfo& samplerInfo);
		static void CreaDefaultResources();

		RendererNew() = delete;
	};
}
