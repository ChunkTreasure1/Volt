#pragma once

#include "Volt/Core/Base.h"

#include "Volt/Rendering/RendererCommon.h"
#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Rendering/RendererStructs.h"

// #TODO_Ivar: Maybe remove from here
#include "Volt/Rendering/RenderGraph/RenderGraph.h"

namespace Volt
{
	namespace RHI
	{
		class Image2D;
		class CommandBuffer;
		class Shader;
		class RenderPipeline;
		class ComputePipeline;

		class UniformBufferSet;
		class StorageBuffer;

		class SamplerState;

		class DescriptorTable;
	}

	class Mesh;
	class Camera;
	class Scene;
	class RenderScene;
	class RenderContext;

	class RenderGraph;
	class RenderGraphBlackboard;

	class RenderGraph::Builder;

	struct SceneRendererSpecification
	{
		std::string debugName;
		glm::uvec2 initialResolution = { 1280, 720 };
		Ref<Scene> scene;
	};

	class SceneRenderer
	{
	public:
		enum class ShadingMode : uint32_t
		{
			Shaded = 0,
			Albedo = 1,
			Normals = 2,
			Metalness = 3,
			Roughness = 4,
			Emissive = 5,

			VisualizeCascades = 6,
			VisualizeLightComplexity = 7
		};

		SceneRenderer(const SceneRendererSpecification& specification);
		~SceneRenderer();

		void OnRenderEditor(Ref<Camera> camera);

		void Resize(const uint32_t width, const uint32_t height);
		inline void SetShadingMode(ShadingMode shadingMode) { m_shadingMode = shadingMode; }
		inline ShadingMode GetShadingMode() const { return m_shadingMode; }

		RefPtr<RHI::Image2D> GetFinalImage();
		RefPtr<RHI::Image2D> GetObjectIDImage();

		// #TODO_Ivar: TEMP, Should not be public!
		void Invalidate();

		const uint64_t GetFrameTotalGPUAllocationSize() const;

	private:
		void OnRender(Ref<Camera> camera);
		void OnRender2(Ref<Camera> camera);

		void BuildMeshPass(RenderGraph::Builder& builder, RenderGraphBlackboard& blackboard, const CullPrimitivesData& cullPrimitivesData);
		void RenderMeshes(RenderContext& context, const RenderGraphPassResources& resources, const RenderGraphBlackboard& blackboard, const CullPrimitivesData& cullPrimitivesData);

		void BuildMeshPass2(RenderGraph::Builder& builder, RenderGraphBlackboard& blackboard);
		void SetupMeshPassConstants(RenderContext& context, const RenderGraphPassResources& resources, const RenderGraphBlackboard& blackboard);

		void SetupFrameData(RenderGraphBlackboard& blackboard, Ref<Camera> camera);

		///// Passes //////
		void UploadUniformBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera);
		void UploadLightBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddExternalResources(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);


		void AddPreDepthPass2(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddObjectIDPass2(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddVisibilityBufferPass2(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);


		void AddPreDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddObjectIDPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddVisibilityBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddClearGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddGenerateMaterialCountsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddCollectMaterialPixelsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddGenerateMaterialIndirectArgsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddGenerateGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, const uint32_t materialId);
		void AddSkyboxPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddShadingPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddFinalCopyPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphResourceHandle srcImage);

		void CreateMainRenderTarget(const uint32_t width, const uint32_t height);

		RefPtr<RHI::Image2D> m_outputImage;
		RefPtr<RHI::Image2D> m_objectIDImage;
		RefPtr<RHI::Image2D> m_previousDepthImage;
		RefPtr<RHI::Image2D> m_previousColorImage;

		Ref<Mesh> m_skyboxMesh;

		bool m_shouldResize = false;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		uint32_t m_resizeWidth = 1280;
		uint32_t m_resizeHeight = 1280;

		RefPtr<RHI::CommandBuffer> m_commandBuffer;
		ShadingMode m_shadingMode = ShadingMode::Shaded;
		PreviousFrameData m_previousFrameData;

		std::atomic<uint64_t> m_frameTotalGPUAllocation;

		///// TEMP /////
		VisibilityVisualization m_visibilityVisualization = VisibilityVisualization::TriangleID;
		////////////////

		Ref<Scene> m_scene;
		SceneEnvironment m_sceneEnvironment;
		DirectionalLightData m_directionalLightData;
	};
}
