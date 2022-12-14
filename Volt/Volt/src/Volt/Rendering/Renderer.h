#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include "Volt/Rendering/Texture/SubTexture2D.h"
#include "Volt/Rendering/RendererStructs.h"
#include "Volt/Rendering/RenderPass.h"
#include "Volt/Scene/Scene.h"

#include <gem/gem.h>

#include <d3d11.h>
#include <wrl.h>

#include <mutex>

using namespace Microsoft::WRL;

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
	class Font;

	struct SpriteVertex;
	struct BillboardVertex;
	struct LineVertex;
	struct TextVertex;

	struct RenderCommand
	{
		SubMesh subMesh;
		gem::mat4 transform;

		std::vector<gem::mat4> boneTransforms;

		Ref<Mesh> mesh;
		Ref<SubMaterial> material;

		float timeSinceCreation = 0.f;
		uint32_t id = 0;
		uint32_t objectBufferId = 0;

		bool castShadows = true;
		bool castAO = true;
	};

	enum class DepthState : uint32_t
	{
		ReadWrite = 0,
		Read = 1,
		None = 2
	};

	enum class RasterizerState : uint32_t
	{
		CullBack = 0,
		CullFront,
		CullNone
	};

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

		struct SectionStatistics
		{
			uint32_t drawCalls = 0;
			uint32_t materialBinds = 0;
			uint32_t vertexIndexBufferBinds = 0;
		};

		struct Statistics
		{
			void Reset()
			{
				drawCalls = 0;
				materialBinds = 0;
				vertexIndexBufferBinds = 0;
				spriteCount = 0;
				billboardCount = 0;
				lineCount = 0;

				perSectionStatistics.clear();
			}

			uint32_t drawCalls = 0;
			uint32_t materialBinds = 0;
			uint32_t vertexIndexBufferBinds = 0;
			uint32_t spriteCount = 0;
			uint32_t billboardCount = 0;
			uint32_t lineCount = 0;

			std::unordered_map<std::string, SectionStatistics> perSectionStatistics;
		};

		struct ProfilingData
		{
			struct SectionPipelineData
			{
				uint64_t vertexCount;
				uint64_t primitiveCount;
				uint64_t psInvocations;
				uint64_t vsInvocations;
			};

			uint32_t currentFrame = 0;
			uint32_t lastFrame = 0;

			uint32_t currentQueryIndex = 0;

			std::array<ComPtr<ID3D11Query>, 2> disjointQuery;
			std::array<ComPtr<ID3D11Query>, 2> beginFrameQuery;
			std::array<ComPtr<ID3D11Query>, 2> endFrameQuery;

			std::array<std::unordered_map<std::string, ComPtr<ID3D11Query>>, 2> sectionTimeQueries;
			std::array<std::unordered_map<std::string, ComPtr<ID3D11Query>>, 2> sectionPipelineQueries;

			std::array<std::vector<std::string>, 2> sectionNames;
			std::array<std::vector<float>, 2> sectionTimes;

			std::unordered_map<std::string, float> sectionAverageTimes;
			std::unordered_map<std::string, float> sectionTotalAverageTimes;

			std::array<std::unordered_map<std::string, SectionPipelineData>, 2> sectionPipelineDatas;
		
			std::array<float, 2> frameGPUTime;
			float frameAverageGPUTime;
			float frameTotalAverageGPUTime;
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

		static void Begin(const std::string& context = "");
		static void End();

		static void BeginPass(const RenderPass& aRenderPass, Ref<Camera> camera, bool shouldClear = true, bool isShadowPass = false, bool isAOPass = false, bool sortByCamera = false);
		static void EndPass();

		static void BeginFullscreenPass(const RenderPass& renderPass, Ref<Camera> camera, bool shouldClear = true);
		static void EndFullscreenPass();

		static void SetFullResolution(const gem::vec2& renderSize);

		static void BeginSection(const std::string& name);
		static void EndSection(const std::string& name);

		static void BeginAnnotatedSection(const std::string& name);
		static void EndAnnotatedSection();

		static void ResetStatistics();

		static void Submit(Ref<Mesh> aMesh, const gem::mat4& aTransform, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);
		static void Submit(Ref<Mesh> aMesh, uint32_t subMeshIndex, const gem::mat4& aTransform, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);
		static void Submit(Ref<Mesh> aMesh, Ref<Material> aMaterial, const gem::mat4& aTransform, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);
		static void Submit(Ref<Mesh> aMesh, Ref<SubMaterial> aMaterial, const gem::mat4& aTransform, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);
		static void Submit(Ref<Mesh> aMesh, const gem::mat4& aTransform, const std::vector<gem::mat4>& aBoneTransforms, uint32_t aId = 0, float aTimeSinceCreation = 0.f, bool castShadows = true, bool castAO = true);

		static void SubmitSprite(Ref<Texture2D> aTexture, const gem::mat4& aTransform, uint32_t id = 0, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });
		static void SubmitSprite(const gem::mat4& aTransform, const gem::vec4& aColor, uint32_t id = 0);

		static void SubmitSprite(const SubTexture2D& aTexture, const gem::mat4& aTransform, uint32_t aId = 0, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });

		static void SubmitBillboard(Ref<Texture2D> aTexture, const gem::vec3& aPosition, const gem::vec3& aScale, uint32_t aId = 0, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });
		static void SubmitBillboard(const gem::vec3& aPosition, const gem::vec3& aScale, const gem::vec4& aColor, uint32_t aId = 0);

		static void SubmitLine(const gem::vec3& aStart, const gem::vec3& aEnd, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });

		static void SubmitLight(const PointLight& pointLight);
		static void SubmitLight(const DirectionalLight& dirLight);

		static void SubmitString(const std::string& aString, const Ref<Font> aFont, const gem::mat4& aTransform, float aMaxWidth, const gem::vec4& aColor = { 1.f });

		static void SubmitDecal(Ref<Material> aMaterial, const gem::mat4& aTransform, uint32_t id = 0, const gem::vec4& aColor = { 1.f, 1.f, 1.f, 1.f });

		static void SubmitResourceChange(const std::function<void()>&& func);

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

		static void SetSceneData(const SceneData& aSceneData);
		static void SetDepthState(DepthState aDepthState);
		static void SetRasterizerState(RasterizerState aRasterizerState);
		static SceneEnvironment GenerateEnvironmentMap(AssetHandle aTextureHandle);

		inline static const DefaultData& GetDefaultData() { return *myDefaultData; }
		inline static const Statistics& GetStatistics() { return myRendererData->statistics; }
		inline static Settings& GetSettings() { return myRendererData->settings; }
		inline static std::unordered_map<std::string, ProfilingData>& GetProfilingData() { return myRendererData->profilingData; }

	private:
		Renderer() = delete;

		static void CreateDepthStates();
		static void CreateRasterizerStates();
		static void CreateDefaultBuffers();
		static void CreateDefaultData();
		static void CreateSpriteData();
		static void CreateBillboardData();
		static void CreateLineData();
		static void CreateTextData();
		static void CreateProfilingData();
		static void CreateInstancingData();
		static void CreateDecalData();

		static void GenerateBRDFLut();

		static void UpdatePerPassBuffers();
		static void UpdatePerFrameBuffers();

		static void UploadObjectData();
		static void RunResourceChanges();

		static void SortRenderCommands();
		static void CollectRenderCommands(const std::vector<RenderCommand>& passCommands, bool shadowPass = false, bool aoPass = false);
		static std::vector<RenderCommand> CullRenderCommands(const std::vector<RenderCommand>& renderCommands, Ref<Camera> camera);

		static void SortRenderCommandsByCamera(std::vector<RenderCommand>& passCommands);

		static ProfilingData& GetContextProfilingData();

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

		struct InstancedRenderCommand
		{
			SubMesh subMesh;

			std::vector<gem::mat4> boneTransforms;
			std::vector<InstanceData> objectDataIds;

			Ref<Mesh> mesh;
			Ref<SubMaterial> material;

			uint32_t count = 0;
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
			std::vector<InstancedRenderCommand> passRenderCommands;
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

			std::vector<RenderCommand> renderCommands;

			std::mutex resourceMutex;
			std::vector<std::function<void()>> resourceChangeQueue;

			std::vector<PointLight> pointLights;
			DirectionalLight directionalLight{};
			Statistics statistics{};
			Settings settings{};
			InstancingData instancingData{};

			std::unordered_map<std::string, ProfilingData> profilingData{};
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
		};

		inline static std::vector<ComPtr<ID3D11DepthStencilState>> myDepthStates;
		inline static std::vector<ComPtr<ID3D11RasterizerState>> myRasterizerStates;

		inline static Scope<DefaultData> myDefaultData;
		inline static Scope<RendererData> myRendererData;
	};
}