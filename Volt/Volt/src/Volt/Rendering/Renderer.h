#pragma once

#include "Volt/Rendering/RendererStructs.h"

#include "Volt/Scene/Scene.h"

#include "Volt/Rendering/BindlessResource.h"
#include <RHIModule/Images/SamplerState.h>
#include <RHIModule/Core/RHICommon.h>

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

		RefPtr<RHI::Image> BRDFLuT;
		RefPtr<RHI::Image> blackCubeTexture;

		Ref<Material> defaultMaterial;

		VT_INLINE void Clear()
		{
			whiteTexture = nullptr;
			BRDFLuT = nullptr;
			blackCubeTexture = nullptr;
			defaultMaterial = nullptr;
		}
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

		template<RHI::TextureFilter min, RHI::TextureFilter mag, RHI::TextureFilter mip, RHI::TextureWrap wrapMode = RHI::TextureWrap::Repeat, RHI::AnisotropyLevel aniso = RHI::AnisotropyLevel::None, RHI::CompareOperator compareOperator = RHI::CompareOperator::None>
		static BindlessResourceRef<RHI::SamplerState> GetSampler()
		{
			RHI::SamplerStateCreateInfo info{};
			info.minFilter = min;
			info.magFilter = mag;
			info.mipFilter = mip;
			info.wrapMode = wrapMode;
			info.anisotropyLevel = aniso;
			info.compareOperator = compareOperator;

			return GetSamplerInternal(info);
		}

	private:
		static BindlessResourceRef<RHI::SamplerState> GetSamplerInternal(const RHI::SamplerStateCreateInfo& samplerInfo);
		static void CreateDefaultResources();
		static void GenerateBRDFLuT();

		Renderer() = delete;
	};
}
