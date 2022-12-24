#pragma once

#include "Volt/Asset/Mesh/SubMesh.h"
#include "Volt/Core/Base.h"
#include "Volt/Scene/Scene.h"

#include "Volt/Rendering/Texture/SubTexture2D.h"
#include "Volt/Rendering/Shader/ShaderCommon.h"

#include "Volt/Rendering/RendererStructs.h"
#include "Volt/Rendering/RenderPass.h"
#include "Volt/Rendering/RendererInfo.h"
#include "Volt/Rendering/RenderCommand.h"

#include <GEM/gem.h>

#include <mutex>

#define VT_THREADED_RENDERING 1

struct ID3D11ShaderResourceView;

namespace Volt
{
	class SamplerState;
	class ConstantBuffer;
	class StructuredBuffer;
	class Mesh;

	class Shader;
	class Material;
	class SubMaterial;
	class ComputePipeline;
	
	class Texture2D;
	class Framebuffer;
	class Camera;
	class VertexBuffer;
	class IndexBuffer;
	class CommandBuffer;
	class Font;

	struct SpriteVertex;
	struct BillboardVertex;
	struct LineVertex;
	struct TextVertex;

	class Renderer
	{
	public:
		struct DefaultData
		{
			Ref<Shader> defaultShader;
			Ref<SubMaterial> defaultMaterial;

			Ref<Texture2D> whiteTexture;
			Ref<Texture2D> emptyNormal;

			Ref<Image2D> brdfLut;
			Ref<Image2D> blackCubeImage;
		};

		struct Settings
		{
			float exposure = 1.f;
			float ambianceMultiplier = 0.5f;
			float bloomWeight = 0.04f;
		};

		static void Initialize();
		static void InitializeBuffers();
		static void Shutdown();

		static void Begin(Context context, const std::string& debugName = "");
		static void End();

		static void BeginPass(const RenderPass& aRenderPass, Ref<Camera> camera, bool shouldClear = true, bool isShadowPass = false, bool isAOPass = false);
		static void EndPass();

		static void BeginFullscreenPass(const RenderPass& renderPass, Ref<Camera> camera, bool shouldClear = true);
		static void EndFullscreenPass();

		static void SetFullResolution(const gem::vec2& renderSize);

		static void BeginSection(const std::string& name);
		static void EndSection(const std::string& name);

		static void Submit(Ref<Mesh> aMesh, const gem::mat4& aTransform, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);
		static void Submit(Ref<Mesh> aMesh, uint32_t subMeshIndex, const gem::mat4& aTransform, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);
		static void Submit(Ref<Mesh> aMesh, Ref<Material> aMaterial, const gem::mat4& aTransform, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);
		static void Submit(Ref<Mesh> aMesh, Ref<SubMaterial> aMaterial, const gem::mat4& aTransform, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);
		static void Submit(Ref<Mesh> aMesh, const gem::mat4& aTransform, const std::vector<gem::mat4>& aBoneTransforms, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);

		static void SubmitSprite(const SubTexture2D& aTexture, const gem::mat4& aTransform, uint32_t aId = 0, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });
		static void SubmitSprite(Ref<Texture2D> aTexture, const gem::mat4& aTransform, uint32_t id = 0, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });
		static void SubmitSprite(const gem::mat4& aTransform, const gem::vec4& aColor, uint32_t id = 0);

		static void SubmitBillboard(Ref<Texture2D> aTexture, const gem::vec3& aPosition, const gem::vec3& aScale, uint32_t aId = 0, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });
		static void SubmitBillboard(const gem::vec3& aPosition, const gem::vec3& aScale, const gem::vec4& aColor, uint32_t aId = 0);

		static void SubmitLine(const gem::vec3& aStart, const gem::vec3& aEnd, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });
		static void SubmitLight(const PointLight& pointLight);
		static void SubmitLight(const DirectionalLight& dirLight);

		static void SubmitString(const std::string& aString, const Ref<Font> aFont, const gem::mat4& aTransform, float aMaxWidth, const gem::vec4& aColor = { 1.f });
		static void SubmitDecal(Ref<Material> aMaterial, const gem::mat4& aTransform, uint32_t id = 0, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });
		static void SubmitResourceChange(const std::function<void()>&& func);

		static void SubmitCustom(std::function<void()>&& func);

		static void SetAmbianceMultiplier(float multiplier); // #TODO: we should probably not do it like this

		static void DrawFullscreenTriangleWithShader(Ref<Shader> aShader);
		static void DrawFullscreenTriangleWithMaterial(Ref<Material> aMaterial);
		static void DrawFullscreenQuadWithShader(Ref<Shader> aShader);
		static void DrawMesh(Ref<Mesh> aMesh, const gem::mat4& aTransform);
		static void DrawMesh(Ref<Mesh> aMesh, const std::vector<gem::mat4>& aBoneTransforms, const gem::mat4& aTransform);
		static void DrawMesh(Ref<Mesh> aMesh, Ref<Material> material, const gem::mat4& aTransform);
		static void DrawMesh(Ref<Mesh> aMesh, Ref<Material> material, uint32_t subMeshIndex, const gem::mat4& aTransform, const std::vector<gem::mat4>& aBoneTransforms = {});

		static void DispatchRenderCommands(bool shadowPass = false, bool aoPass = false);
		static void DispatchRenderCommandsInstanced();

		static void DispatchLines();
		static void DispatchText();
		static void DispatchSpritesWithShader(Ref<Shader> aShader);
		static void DispatchBillboardsWithShader(Ref<Shader> aShader);
		static void DispatchDecalsWithShader(Ref<Shader> aShader);
		static void DispatchSpritesWithMaterial(Ref<Material> aMaterial);

		static void ExecuteFullscreenPass(Ref<ComputePipeline> aComputePipeline, Ref<Framebuffer> aFramebuffer);

		static void SyncAndWait();

		static void BindTexturesToStage(ShaderStage stage, const std::vector<Ref<Image2D>>& textures, const uint32_t startSlot);
		static void ClearTexturesAtStage(ShaderStage stage, const uint32_t startSlot, const uint32_t count);

		static void SetSceneData(const SceneData& aSceneData);

		static SceneEnvironment GenerateEnvironmentMap(AssetHandle aTextureHandle);

		inline static const DefaultData& GetDefaultData() { return *myDefaultData; }
		inline static Settings& GetSettings() { return myRendererData->settings; }

	private:
		Renderer() = delete;

		static void CreateDefaultBuffers();
		static void CreateDefaultData();
		static void CreateSpriteData();
		static void CreateBillboardData();
		static void CreateLineData();
		static void CreateTextData();
		static void CreateInstancingData();
		static void CreateDecalData();
		static void CreateCommandBuffers();

		static void GenerateBRDFLut();

		static void UpdatePerPassBuffers();
		static void UpdatePerFrameBuffers();
		static void UploadObjectData(std::vector<SubmitCommand>& submitCommands);

		static void RunResourceChanges();
		static void SortSubmitCommands(std::vector<SubmitCommand>& submitCommands);
		static void CollectSubmitCommands(const std::vector<SubmitCommand>& passCommands, std::vector<InstancedSubmitCommand>& instanceCommands, bool shadowPass = false, bool aoPass = false);
		static std::vector<SubmitCommand> CullRenderCommands(const std::vector<SubmitCommand>& renderCommands, Ref<Camera> camera);

		///// Threading //////
		static void DispatchRenderCommandsInstancedInternal();
		static void DrawMeshInternal(Ref<Mesh> aMesh, Ref<Material> material, uint32_t subMeshIndex, const gem::mat4& aTransform, const std::vector<gem::mat4>& aBoneTransforms = {});

		static void RT_Execute();
		//////////////////////

		static void DrawFullscreenTriangleWithShaderInternal(Ref<Shader> shader);
		static void DrawFullscreenTriangleWithMaterialInternal(Ref<Material> aMaterial);
		static void DrawFullscreenQuadWithShaderInternal(Ref<Shader> aShader);

		static void BeginInternal(Context context, const std::string& debugName);
		
		static void BeginPassInternal(const RenderPass& aRenderPass, Ref<Camera> aCamera, bool aShouldClear, bool aIsShadowPass, bool aIsAOPass);
		static void EndPassInternal();

		static void BeginFullscreenPassInternal(const RenderPass& renderPass, Ref<Camera> camera, bool shouldClear /* = true */);
		static void EndFullscreenPassInternal();

		struct Samplers
		{
			Ref<SamplerState> linearWrap;
			Ref<SamplerState> linearPointWrap;

			Ref<SamplerState> pointWrap;
			Ref<SamplerState> pointLinearWrap;

			Ref<SamplerState> linearClamp;
			Ref<SamplerState> linearPointClamp;

			Ref<SamplerState> pointClamp;
			Ref<SamplerState> pointLinearClamp;

			Ref<SamplerState> anisotropic;
			Ref<SamplerState> shadow;
		};

		struct DecalRenderCommand
		{
			gem::mat4 transform;
			gem::vec4 color;
			Ref<Material> material;
			uint32_t id;
		};
		
		struct SpriteData
		{
			inline static constexpr uint32_t MAX_QUADS = 5000;
			inline static constexpr uint32_t MAX_VERTICES = MAX_QUADS * 4;
			inline static constexpr uint32_t MAX_INDICES = MAX_QUADS * 6;

			Ref<VertexBuffer> vertexBuffer;
			Ref<IndexBuffer> indexBuffer;
			Ref<Texture2D> textureSlots[32];

			gem::vec4 vertices[4];
			gem::vec2 texCoords[4];
			uint32_t indexCount = 0;
			uint32_t textureSlotIndex = 1;

			SpriteVertex* vertexBufferBase = nullptr;
			SpriteVertex* vertexBufferPtr = nullptr;
		};

		struct LineData
		{
			inline static constexpr uint32_t MAX_LINES = 200000;
			inline static constexpr uint32_t MAX_VERTICES = MAX_LINES * 2;
			inline static constexpr uint32_t MAX_INDICES = MAX_LINES * 2;

			Ref<VertexBuffer> vertexBuffer;
			Ref<IndexBuffer> indexBuffer;
			Ref<Shader> shader;

			LineVertex* vertexBufferBase = nullptr;
			LineVertex* vertexBufferPtr = nullptr;

			uint32_t indexCount = 0;
		};

		struct BillboardData
		{
			inline static constexpr uint32_t MAX_BILLBOARDS = 50000;

			Ref<VertexBuffer> vertexBuffer;
			Ref<Texture2D> textureSlots[32];

			uint32_t textureSlotIndex = 1;
			uint32_t vertexCount = 0;

			BillboardVertex* vertexBufferBase = nullptr;
			BillboardVertex* vertexBufferPtr = nullptr;
		};

		struct TextData
		{
			inline static constexpr uint32_t MAX_QUADS = 10000;
			inline static constexpr uint32_t MAX_VERTICES = MAX_QUADS * 4;
			inline static constexpr uint32_t MAX_INDICES = MAX_QUADS * 6;

			Ref<VertexBuffer> vertexBuffer;
			Ref<IndexBuffer> indexBuffer;
			Ref<Texture2D> textureSlots[32];
			Ref<Shader> shader;

			gem::vec4 vertices[4];
			uint32_t indexCount = 0;
			uint32_t textureSlotIndex = 0;

			TextVertex* vertexBufferBase = nullptr;
			TextVertex* vertexBufferPtr = nullptr;
		};

		struct InstancingData
		{
			Ref<VertexBuffer> instancedVertexBuffer;
			std::vector<InstancedSubmitCommand> passRenderCommands;
		};

		struct DecalData
		{
			std::vector<DecalRenderCommand> renderCommands;
			Ref<Mesh> cubeMesh;
		};

		struct RendererData
		{
			inline static constexpr uint32_t MAX_OBJECTS_PER_FRAME = 5000;
			inline static constexpr uint32_t MAX_BONES_PER_MESH = 128;
			inline static constexpr uint32_t MAX_CULL_THREAD_COUNT = 4;
			inline static constexpr uint32_t MAX_POINT_LIGHTS = 1024;

			///// Threading /////
			Ref<CommandBuffer> currentCPUBuffer = nullptr;
			Ref<CommandBuffer> currentGPUBuffer = nullptr;

			Ref<CommandBuffer> commandBuffers[2];
			////////////////////
			std::vector<SubmitCommand> renderCommands;

			std::mutex resourceMutex;
			std::vector<std::function<void()>> resourceChangeQueue;

			std::vector<PointLight> pointLights;
			DirectionalLight directionalLight{};
			Settings settings{};
			InstancingData instancingData{};

			std::string currentContext;

			Samplers samplers{};
			RenderPass currentPass{};
			Ref<Camera> currentPassCamera;
			SceneData sceneData;

			SpriteData spriteData{};
			BillboardData billboardData{};
			LineData lineData{};
			TextData textData{};

			DecalData decalData{};

			// Fullscreen quad
			Ref<VertexBuffer> quadVertexBuffer;
			Ref<IndexBuffer> quadIndexBuffer;

			std::unordered_map<AssetHandle, SceneEnvironment> environmentCache;
			gem::vec2 fullRenderSize = { 1.f, 1.f };

			///// Threading /////
			std::atomic_bool isRunning = false;
			bool renderReady = false;
			bool updateReady = false;

			std::mutex renderMutex;
			std::condition_variable syncVariable;
			std::thread renderThread;
			/////////////////////
		};

		inline static Scope<DefaultData> myDefaultData;
		inline static Scope<RendererData> myRendererData;
	};
}