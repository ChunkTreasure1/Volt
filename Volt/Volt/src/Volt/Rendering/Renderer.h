#pragma once

#include "Volt/Rendering/RendererStructs.h"
#include "Volt/Rendering/Resources/GlobalResource.h"

#include "Volt/Scene/Scene.h"

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
	class Material;
	class ShaderRuntimeValidator;

	struct DefaultResources
	{
		Ref<Texture2D> whiteTexture;

		Ref<RHI::Image2D> BRDFLuT;
		Ref<RHI::Image2D> blackCubeTexture;

		Ref<Material> defaultMaterial;
	};

	class Mesh;
	class Renderer
	{
	public:
		static void PreInitialize();
		static void Initialize();
		static void Shutdown();

		static void Flush();
		static const uint32_t GetFramesInFlight();
		static void DestroyResource(std::function<void()>&& function);

		static const DefaultResources& GetDefaultResources();
		static SceneEnvironment GenerateEnvironmentTextures(AssetHandle baseTextureHandle);

#ifndef VT_DIST
		static ShaderRuntimeValidator& GetRuntimeShaderValidator();
#endif

		static void Update();
		static void EndOfFrameUpdate();

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
		static void CreateDefaultResources();
		static void GenerateBRDFLuT();

		Renderer() = delete;
	};
}
