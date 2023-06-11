#pragma once

#include "Volt/Asset/Mesh/SubMaterial.h"

#include "Volt/Core/Threading/ThreadPool.h"

#include "Volt/Rendering/SceneRendererSettings.h"

#include "Volt/Rendering/RendererCommon.h"
#include "Volt/Rendering/RendererStructs.h"

#include "Volt/Rendering/GlobalDescriptorSet.h"
#include "Volt/Rendering/RenderPipeline/RenderPipeline.h"
#include "Volt/Rendering/FrameGraph/TransientResourceSystem.h"
#include "Volt/Rendering/FrameGraph/CommandBufferCache.h"

namespace Volt
{
	class Mesh;
	class Scene;
	class Material;
	class Camera;
	class Image3D;
	class RenderPipeline;
	class UniformBufferSet;
	class VulkanFramebuffer;
	class CommandBuffer;
	class ComputePipeline;
	class ShaderStorageBufferSet;
	class RayTracedSceneRenderer;

	class GlobalDescriptorSet;
	class VertexBufferSet;

	class PostProcessingStack;

	struct VulkanRenderPass;
	struct LineVertex;
	struct BillboardVertex;

	struct SceneRendererSpecification
	{
		std::string debugName;
		glm::uvec2 initialResolution = { 1280, 720 };
		Weak<Scene> scene;

		bool enablePostProcess = true;
		bool renderToSwapchain = false;
	};

	struct SceneRendererStatistics
	{
		uint64_t triangleCount = 0;
		RenderPipelineStatistics pipelineStatistics;

		inline void Reset()
		{ 
			pipelineStatistics = {};
			triangleCount = 0;
		}
	};

	class SceneRenderer
	{
	public:
		inline static constexpr uint32_t BLOOM_MIP_COUNT = 5;
		inline static constexpr uint32_t MAX_SPOT_LIGHT_SHADOWS = 8;
		inline static constexpr uint32_t MAX_POINT_LIGHT_SHADOWS = 8;

		inline static constexpr uint32_t VOLUMETRIC_FOG_WIDTH = 160;
		inline static constexpr uint32_t VOLUMETRIC_FOG_HEIGHT = 90;
		inline static constexpr uint32_t VOLUMETRIC_FOG_DEPTH = 92;

		struct Timestamp
		{
			std::string label;
			float time;
		};

		struct PerThreadData
		{
			std::vector<SubmitCommand> submitCommands;
			std::vector<IndirectBatch> indirectBatches;
			std::vector<ParticleBatch> particleBatches;

			std::vector<SubmitCommand> outlineCommands;
			std::vector<LineCommand> lineCommands;
			std::vector<BillboardCommand> billboardCommands;
			std::vector<DecalCommand> decalCommands;

			Ref<PostProcessingStack> postProcessingStack;

			DirectionalLight directionalLight{};
			std::vector<PointLight> pointLights{};
			std::vector<SpotLight> spotLights{};
			std::vector<SphereLight> sphereLights{};
			std::vector<RectangleLight> rectangleLights{};

			uint32_t particleCount = 0;
			uint32_t animationBoneCount = 0;
			uint32_t vertexColorCount = 0;
			uint32_t spotLightShadowCount = 0;
			uint32_t pointLightShadowCount = 0;

			inline void Clear()
			{
				submitCommands.clear();
				lineCommands.clear();
				billboardCommands.clear();

				indirectBatches.clear();
				particleBatches.clear();

				pointLights.clear();
				spotLights.clear();
				sphereLights.clear();
				rectangleLights.clear();
				decalCommands.clear();

				directionalLight = {};

				particleCount = 0;
				animationBoneCount = 0;
				spotLightShadowCount = 0;
				pointLightShadowCount = 0;
				vertexColorCount = 0;
			}
		};

		SceneRenderer(const SceneRendererSpecification& specification, const SceneRendererSettings& initialSettings = {});
		~SceneRenderer();

		void OnRenderEditor(Ref<Camera> camera);
		void OnRenderRuntime();

		void Resize(uint32_t width, uint32_t height);

		void SubmitOutlineMesh(Ref<Mesh> mesh, const glm::mat4& transform);
		void SubmitOutlineMesh(Ref<Mesh> mesh, uint32_t subMeshIndex, const glm::mat4& transform);
		void ClearOutlineCommands();

		void SubmitMesh(Ref<Mesh> mesh, Ref<Material> material, const glm::mat4& transform, float timeSinceCreation, float randomValue, uint32_t id);
		void SubmitMesh(Ref<Mesh> mesh, Ref<Material> material, const glm::mat4& transform, const std::vector<glm::mat4>& boneTransforms, float timeSinceCreation, float randomValue, uint32_t id);

		Ref<Image2D> GetFinalImage();
		Ref<Image2D> GetIDImage();

		inline SceneRendererSettings& GetSettings() { return mySettings; }

		void ApplySettings();

		void UpdateRayTracingScene(); // #TODO_Ivar: Remove!

		const std::vector<Timestamp> GetTimestamps();
		const SceneRendererStatistics& GetStatistics() const;

		void SetHideStaticMeshes(bool active) { myShouldHideMeshes = active; };

	private:
		enum class DrawCullMode : uint32_t
		{
			None = 0,
			Frustum,
			AABB
		};

		struct EnvironmentSettings
		{
			Ref<Image2D> irradianceMap;
			Ref<Image2D> radianceMap;

			float turbidity = 0.f;
			float azimuth = 0.f;
			float inclination = 0.f;

			float intensity = 1.f;
		};

		struct TimestampInfo
		{
			std::string label;
			uint32_t id;
		};

		struct IndirectPassParams
		{
			MaterialFlag materialFlags = MaterialFlag::All;
			DrawCullMode cullMode = DrawCullMode::Frustum;

			glm::vec3 aabbMin = 0.f;
			glm::vec3 aabbMax = 0.f;
		};

		struct LineData
		{
			inline static constexpr uint32_t MAX_LINES = 200000;
			inline static constexpr uint32_t MAX_VERTICES = MAX_LINES * 2;
			inline static constexpr uint32_t MAX_INDICES = MAX_LINES * 2;

			Ref<VertexBufferSet> vertexBuffer;
			Ref<RenderPipeline> renderPipeline;

			LineVertex* vertexBufferBase = nullptr;
			LineVertex* vertexBufferPtr = nullptr;
			uint32_t vertexCount = 0;
		};

		struct BillboardData
		{
			inline static constexpr uint32_t MAX_BILLBOARDS = 500000;

			Ref<VertexBufferSet> vertexBuffer;
			Ref<RenderPipeline> renderPipeline;

			BillboardVertex* vertexBufferBase = nullptr;
			BillboardVertex* vertexBufferPtr = nullptr;
			uint32_t vertexCount = 0;
		};

		void BeginTimestamp(const std::string& label);
		void EndTimestamp();

		void OnRender(Ref<Camera> camera);

		void CreateResources();
		void CreateBuffers();
		void CreateGlobalDescriptorSets();
		void UpdateGlobalDescriptorSets();

		void CreateLineData();
		void CreateBillboardData();

		void UpdateBuffers(Ref<Camera> camera);
		void FetchCommands(Ref<Camera> camera);
		void FetchMeshCommands();
		void FetchAnimatedMeshCommands();
		void FetchParticles(Ref<Camera> camera);

		void ResizeBuffersIfNeeded();

		void SortSubmitCommands();
		void PrepareForIndirectDraw();
		void PrepareLineBuffer();
		void PrepareBillboardBuffer();

		void UploadIndirectDrawCommands();
		void UploadObjectData();
		void UploadPerFrameData();

		void SortAndUploadParticleData(Ref<Camera> camera);

		void UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Ref<UniformBufferSet> uniformBufferSet);
		void UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Ref<ShaderStorageBufferSet> storageBufferSet);
		void UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Weak<Image2D> image);
		void UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Weak<Image3D> image);
		void UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Weak<Image2D> image, VkImageLayout targetLayout);
		void UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, std::vector<Weak<Image2D>> images, VkImageLayout targetLayout);

		void ExecuteFrame();
		void EvaluateTimestamps();

		const IndirectPass& GetOrCreateIndirectPass(Ref<CommandBuffer> commandBuffer, const IndirectPassParams& params = {});

		inline PerThreadData& GetCPUData() { return myPerThreadDatas[myCurrentCPUData]; }
		inline PerThreadData& GetGPUData() { return myPerThreadDatas[myCurrentGPUData]; }

		///// Render Passes /////
		void AddDirectionalShadowPass(FrameGraph& frameGraph);
		void AddSpotlightShadowPasses(FrameGraph& frameGraph);
		void AddPointLightShadowPasses(FrameGraph& frameGraph);

		void UpdatePBRDescriptors(FrameGraph& frameGraph);

		void AddIDPass(FrameGraph& frameGraph);
		void AddPreDepthPass(FrameGraph& frameGraph);

		void AddPreethamSkyPass(FrameGraph& frameGraph);
		void AddSkyboxPass(FrameGraph& frameGraph);

		void AddGBufferPass(FrameGraph& frameGraph);
		void AddDecalPass(FrameGraph& frameGraph);
		void AddDeferredShadingPass(FrameGraph& frameGraph);

		void AddForwardOpaquePass(FrameGraph& frameGraph);

		void AddForwardSSSPass(FrameGraph& frameGraph);
		void AddSSSBlurPass(FrameGraph& frameGraph);
		void AddSSSCompositePass(FrameGraph& frameGraph);

		void AddForwardTransparentPass(FrameGraph& frameGraph);
		void AddTransparentCompositePass(FrameGraph& frameGraph);

		void AddParticlesPass(FrameGraph& frameGraph);
		void AddBillboardPass(FrameGraph& frameGraph);

		void AddLuminosityPass(FrameGraph& frameGraph);

		void AddPostProcessingStackPasses(FrameGraph& frameGraph);

		void AddACESPass(FrameGraph& frameGraph);
		void AddGammaCorrectionPass(FrameGraph& frameGraph);
		void AddLightCullPass(FrameGraph& frameGraph);

		void AddGridPass(FrameGraph& frameGraph);

		void AddCopyHistoryPass(FrameGraph& frameGraph);

		void RenderUI();

		void ClearCountBuffer(const IndirectPass& pass, Ref<CommandBuffer> commandBuffer);
		void CullRenderCommands(const IndirectPass& pass, const IndirectPassParams& params, Ref<CommandBuffer> commandBuffer);

		inline static constexpr uint32_t MAX_OBJECT_COUNT = 20000;
		inline static constexpr uint32_t MAX_POINT_LIGHTS = 1024;
		inline static constexpr uint32_t MAX_SPOT_LIGHTS = 1024;
		inline static constexpr uint32_t MAX_SPHERE_LIGHTS = 1024;
		inline static constexpr uint32_t MAX_RECTANGLE_LIGHTS = 1024;
		inline static constexpr uint32_t MAX_ANIMATION_BONES = 8192;
		inline static constexpr uint32_t MAX_PARTICLES = 1'000'000;

		SceneRendererSpecification mySpecification{};
		SceneRendererSettings mySettings{};
		SceneRendererStatistics myStatistics{};

		bool myShouldHideMeshes = false;
		bool myShouldResize = false;
		glm::uvec2	myRenderSize = { 1 };
		glm::uvec2	myScaledSize = { 1 };
		glm::uvec2 myOriginalSize = { 1 };

		Weak<Scene> myScene;

		Ref<UniformBufferSet> myUniformBufferSet;
		Ref<ShaderStorageBufferSet> myShaderStorageBufferSet;

		Ref<CommandBuffer> myCommandBuffer;

		Scope<RayTracedSceneRenderer> myRayTracedSceneRenderer;

		std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>> myGlobalDescriptorSets;
		std::vector<std::vector<TimestampInfo>> myTimestamps;

		///// Mesh Passes /////
		std::vector<IndirectPass> myIndirectPasses;
		std::atomic_uint32_t myCurrentIndirectPass = 0;

		///// Settings /////
		EnvironmentSettings myEnvironmentSettings;
		GTAOSettings myGTAOSettings;
		VolumetricFogSettings myVolumetricFogSettings;

		///// Frame Graph /////
		Ref<Image2D> myOutputImage;
		Ref<Image2D> myIDImage;
		Ref<Image2D> myHistoryDepth;
		Ref<Image2D> myHistoryColor;

		Ref<Image3D> myVolumetricFogInjectImage;
		Ref<Image3D> myVolumetricFogRayMarchImage;

		TransientResourceSystem myTransientResourceSystem;

		Ref<RenderPipeline> myGBufferPipeline;
		Ref<RenderPipeline> myPreDepthPipeline;
		Ref<RenderPipeline> myIDPipeline;
		Ref<RenderPipeline> myDirectionalShadowPipeline;
		Ref<RenderPipeline> mySpotLightShadowPipeline;
		Ref<RenderPipeline> myPointLightShadowPipeline;
		Ref<RenderPipeline> myOutlineGeometryPipeline;

		Ref<ComputePipeline> myDepthReductionPipeline;

		Ref<ComputePipeline> myLightCullPipeline;
		Ref<ComputePipeline> myDeferredShadingPipeline;

		Ref<ComputePipeline> myPreethamPipeline;
		Ref<ComputePipeline> myACESPipeline;
		Ref<ComputePipeline> myGammaCorrectionPipeline;

		Ref<ComputePipeline> myLuminosityPipeline;

		Ref<ComputePipeline> myBloomDownsamplePipeline;
		Ref<ComputePipeline> myBloomUpsamplePipeline;
		Ref<ComputePipeline> myBloomCompositePipeline;

		Ref<ComputePipeline> myGTAOPrefilterPipeline;
		Ref<ComputePipeline> myGTAOMainPassPipeline;
		std::vector<Ref<ComputePipeline>> myGTAODenoisePipelines;

		Ref<ComputePipeline> myOutlineCompositePipeline;

		Ref<ComputePipeline> myGenerateMotionVectorsPipeline;
		Ref<ComputePipeline> myTAAPipeline;
		Ref<ComputePipeline> myFXAAPipeline;
		Ref<ComputePipeline> myAACompositePipeline;

		Ref<ComputePipeline> myVolumetricFogInjectPipeline;
		Ref<ComputePipeline> myVolumetricFogRayMarchPipeline;

		Ref<ComputePipeline> mySSSBlurPipeline[2];
		Ref<ComputePipeline> mySSSCompositePipeline;

		Ref<ComputePipeline> mySSRPipeline;
		Ref<ComputePipeline> mySSRCompositePipeline;

		Ref<Material> myGridMaterial;
		Ref<Material> myDefaultParticleMaterial;
		Ref<Material> myTransparentCompositeMaterial;
		Ref<Material> myDecalMaterial;

		Ref<Material> myOutlineJumpFloodInitMaterial;
		Ref<Material> myOutlineJumpFloodPassMaterial;

		///// Draw calls /////
		PerThreadData myPerThreadDatas[2];
		uint32_t myCurrentGPUData = 0;
		uint32_t myCurrentCPUData = 0;

		////// Resources //////
		Ref<Mesh> mySkyboxMesh;
		Ref<Mesh> myDecalMesh;
		Ref<Camera> myCurrentCamera;
		LineData myLineData;
		BillboardData myBillboardData;

		////// Timestamps //////
		std::mutex myTimestampsMutex;
		std::vector<Timestamp> myTimestampsResult;
		RenderPipelineStatistics myRenderPipelineStatistics;

		uint64_t myFrameIndex = 0;
		glm::mat4 myPreviousViewProjection = 1.f;
		glm::vec2 myPreviousJitter = 0.f;
		glm::vec2 myLastJitter = 0.f;

		////// Threading //////
		ThreadPool myRenderThreadPool;
		CommandBufferCache myCommandBufferCache;
		std::mutex myRenderingMutex;
	};
}
