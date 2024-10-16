#pragma once

#include "Volt/Rendering/RendererStructs.h"

#include <AssetSystem/AssetHandle.h>
#include <RenderCore/Resources/BindlessResource.h>

#include <RHIModule/Images/SamplerState.h>
#include <RHIModule/Core/RHICommon.h>

#include <SubSystem/SubSystem.h>
#include <EventSystem/EventListener.h>

#include <CoreUtilities/Containers/FunctionQueue.h>

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
		Ref<Material> defaultMaterial;

		RefPtr<RHI::Image> BRDFLuT;
		RefPtr<RHI::Image> blackCubeTexture;

		VT_INLINE void Clear()
		{
			whiteTexture = nullptr;
			BRDFLuT = nullptr;
			blackCubeTexture = nullptr;
			defaultMaterial = nullptr;
		}
	};

	class Mesh;
	class ShaderMap;

	class AppPostFrameUpdateEvent;
	class AppPreRenderEvent;

	class Renderer : public SubSystem, public EventListener
	{
	public:
		Renderer();
		~Renderer() override;

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void Initialize() override;
		void Shutdown() override;

		static const uint32_t GetFramesInFlight();
		static void DestroyResource(std::function<void()>&& function);

		static const DefaultResources& GetDefaultResources();
		static SceneEnvironment GenerateEnvironmentTextures(AssetHandle baseTextureHandle);

#ifndef VT_DIST
		static ShaderRuntimeValidator& GetRuntimeShaderValidator();
#endif

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

			return s_instance->GetSamplerInternal(info);
		}

		VT_DECLARE_SUBSYSTEM("{2E420D68-01AC-47D5-B7F4-F31F13D57ABF}"_guid);

	private:
		bool OnEndOfFrameUpdate(AppPostFrameUpdateEvent& event);
		bool OnPreRenderEvent(AppPreRenderEvent& event);

		BindlessResourceRef<RHI::SamplerState> GetSamplerInternal(const RHI::SamplerStateCreateInfo& samplerInfo);
		void CreateDefaultResources();
		void GenerateBRDFLuT();
		void LoadShaders();

		inline static Renderer* s_instance = nullptr;

		DefaultResources m_defaultResources;

		Scope<ShaderMap> m_shaderMap;
		Scope<BindlessResourcesManager> m_bindlessResourcesManager;

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION
		Scope<ShaderRuntimeValidator> m_shaderValidator;
#endif

		Vector<FunctionQueue> m_deletionQueue;
		vt::map<size_t, BindlessResourceRef<RHI::SamplerState>> m_samplers;
	};
}
