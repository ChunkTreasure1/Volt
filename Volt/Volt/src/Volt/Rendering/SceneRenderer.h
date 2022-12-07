#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Core/Base.h"
#include "Volt/Rendering/RenderPass.h"
#include "Volt/Scene/Scene.h"

#include "Volt/Components/Components.h"

#include <functional>
#include <thread>
#include <mutex>

namespace Volt
{
	class Camera;
	class Scene;
	class Framebuffer;
	class Mesh;
	class ComputePipeline;
	class Shader;
	class ConstantBuffer;
	class StructuredBuffer;

	class SceneRenderer
	{
	public:
		SceneRenderer(Ref<Scene> aScene, const std::string& context = "None", bool isLowRes = false);

		void OnRenderEditor(Ref<Camera> aCamera);
		void OnRenderRuntime();

		void Resize(uint32_t width, uint32_t height);
		void AddForwardCallback(std::function<void(Ref<Scene>, Ref<Camera>)>&& callback);
		void AddExternalPassCallback(std::function<void(Ref<Scene>, Ref<Camera>)>&& callback);
		void UpdateVignetteSettings();

		Ref<Framebuffer> GetFinalFramebuffer();
		Ref<Framebuffer> GetFinalObjectFramebuffer();
		Ref<Framebuffer> GetSelectionFramebuffer();

		inline auto& GetHBAOSettings() { return myHBAOSettings; }
		inline auto& GetBloomSettings() { return myBloomSettings; }
		inline auto& GetFXAASettings() { return myFXAASettings; }
		inline auto& GetVignetteSettings() { return myVignetteSettings; }
		inline auto& GetGammaSettingss() { return myGammaSettings;  }

		inline const auto& GetAllFramebuffers() const { return myFramebuffers; }

	private:
		struct HBAOData
		{
			int uvOffsetIndex;
			float negInvR2;
			float multiplier;
			float powExponent;

			gem::vec4 float2Offsets[16];
			gem::vec4 jitters[16];
			gem::vec4 perspectiveInfo;

			gem::vec2 inverseQuarterSize;
			float radiusToScreen;
			float NdotVBias;

			gem::vec2 invResDirection;
			float sharpness;
			float padding;
		};

		struct HBAOSettings
		{
			bool enabled = true;
			float radius = 1.f;
			float intensity = 1.5f;
			float bias = 0.35f;
		};

		struct BloomSettings
		{
			bool enabled = true;
		};

		struct FXAASettings
		{
			bool enabled = true;
		};

		struct VignetteSettings
		{
			bool enabled = true;

			float width = 1.5f;
			float sharpness = 0.8f;
			gem::vec3 color = { 0, 0, 0 };
		};

		struct ColorGradeSettings
		{
			bool enabled = false;
		};

		struct GammaSettings
		{
			bool enabled = true;
		};

		struct VoxelSceneData
		{
			gem::vec3 center = { 0.f, 0.f, 0.f };
			float voxelSize = 100.f;

			uint32_t resolution = 128;
			uint32_t padding[3];
		};

		struct ColorGradeData
		{
			float unsharpenMask = 0.f;
			float contrastPivot = 0.435f;
			float contrast = 1.f;
			float saturation = 1.f;

			float hue = 0.f;
			float temperature = 0.f;
			float gain = 0.f;
			float lift = 0.f;

			float offset = 0.f;
			float gamma = 0.f;

			gem::vec2 padding;
		};

		struct HeightFogData
		{
			gem::vec3 fogColor = { 1.f, 1.f, 1.f };
			float strength = 1.f;

			float fogMaxY = 200.f;
			float fogMinY = 0.f;
			gem::vec2 padding;
		};

		struct VoxelType
		{
			uint32_t colorMask;
			uint32_t normalMask;

			uint32_t padding[2];
		};

		struct SkyboxData
		{
			float textureLod = 0.f;
			float intensity = 1.f;

			gem::vec2 padding;
		};

		void OnRender(Ref<Camera> aCamera);

		void CreatePasses(bool isLowRes);

		void ShadingPass(const SceneEnvironment& sceneEnv);

		void PostProcessPasses(Ref<Camera> aCamera);
		void ResetPostProcess();

		void BloomPass(Ref<Image2D> sourceImage);
		void HBAOPass(Ref<Camera> aCamera);
		void VoxelPass(Ref<Camera> aCamera);

		void CollectStaticMeshCommands();
		void CollectAnimatedMeshCommands();

		const uint32_t myMaxCollectionThreads = 4;

		bool myShouldResize = false;
		gem::vec2ui myResizeSize = { 1, 1 };
		std::string myContext;
		
		std::vector<std::pair<Volt::AssetHandle, Wire::EntityId>> myHighlightedMeshes;
		std::vector<std::tuple<Volt::AssetHandle, Wire::EntityId, AnimatedCharacterComponent>> myHighlightedAnimatedMeshes;

		///// Settings /////
		HBAOSettings myHBAOSettings;
		BloomSettings myBloomSettings;
		FXAASettings myFXAASettings;
		VignetteSettings myVignetteSettings;
		ColorGradeSettings myColorGradeSettings;
		ColorGradeData myColorGradeData;
		GammaSettings myGammaSettings;
		////////////////////

		Ref<Scene> myScene;
		Ref<Material> myVignetteMaterial;
		Ref<Shader> myBillboardShader;

		RenderPass myForwardPass;
		RenderPass myDeferredPass;
		RenderPass myDecalPass;
		RenderPass myShadingPass;
		RenderPass myDirectionalShadowPass;
		RenderPass myFXAAPass;
		RenderPass myVignettePass;
		RenderPass myGammaCorrectionPass;
		RenderPass myPreDepthPass;

		RenderPass myHeightFogPass;
		Ref<ConstantBuffer> myHeightFogBuffer;
		HeightFogData myHeightFogData;

		///// Skybox /////
		RenderPass mySkyboxPass;
		Ref<Mesh> mySkyboxMesh;
		Ref<ConstantBuffer> mySkyboxBuffer;
		SkyboxData mySkyboxData;
		//////////////////

		///// Bloom /////
		const uint32_t myBloomMipCount = 6;
		std::vector<RenderPass> myBloomDownsamplePasses;
		std::vector<RenderPass> myBloomUpsamplePasses;
		RenderPass myBloomCompositePass;
		Ref<ComputePipeline> myBloomDownsamplePipeline;
		Ref<ComputePipeline> myBloomUpsamplePipeline;
		Ref<ConstantBuffer> myBloomUpsampleBuffer;
		/////////////////

		///// HABO /////
		RenderPass myDeinterleavingPass[2];
		RenderPass myReinterleavingPass;
		RenderPass myHBAOBlurPass[2];
		RenderPass myAOCompositePass;
		HBAOData myHBAOData;
		Ref<ConstantBuffer> myHBAOBuffer;

		Ref<ComputePipeline> myHBAOPipeline;
		RenderPass myHBAOPass;
		////////////////

		///// Outline /////
		RenderPass myHighlightedGeometryPass;
		RenderPass myJumpFloodInitPass;
		RenderPass myJumpFloodCompositePass;

		RenderPass myJumpFloodPass[2];
		Ref<ConstantBuffer> myJumpFloodBuffer;
		///////////////////

		///// Point light shadow /////
		RenderPass myPointLightPass;

		Ref<ConstantBuffer> myPointLightBuffer;
		//////////////////////////////
		std::vector<std::function<void(Ref<Scene>, Ref<Camera>)>> myForwardRenderCallbacks;
		std::vector<std::function<void(Ref<Scene>, Ref<Camera>)>> myExternalPassRenderCallbacks;

		std::unordered_map<uint32_t, std::pair<std::string, Ref<Framebuffer>>> myFramebuffers;

		RenderPass myVoxelPass;
		VoxelSceneData myVoxelData;
		Ref<ConstantBuffer> myVoxelBuffer;
		Ref<StructuredBuffer> myVoxelResultBuffer;

		///// Collection /////
		std::vector<std::thread> myCollectionThreads;
		std::vector<bool> myIsThreadsIdle;

		std::vector<std::function<void()>> myCollectionJobs;
		std::mutex myCollectionMutex;

		std::atomic_bool myIsRunning = true;
		//////////////////////
	};
}