#pragma once

#include "Volt/Core/Base.h"

#include "Volt/Rendering/RendererCommon.h"
#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Rendering/RendererStructs.h"

// #TODO_Ivar: Maybe remove from here
#include <RenderCore/RenderGraph/RenderGraph.h>

#include <RHIModule/Buffers/CommandBufferSet.h>

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
			AO = 6,
		};

		enum class VisualizationMode : uint32_t
		{
			None = 0,
			VisualizeCascades = 1,
			VisualizeLightComplexity = 2,
			VisualizeMeshSDF = 3
		};

		SceneRenderer(const SceneRendererSpecification& specification);
		~SceneRenderer();

		void OnRenderEditor(Ref<Camera> camera);

		void Resize(const uint32_t width, const uint32_t height);
		inline void SetShadingMode(ShadingMode shadingMode) { m_shadingMode = shadingMode; }
		inline ShadingMode GetShadingMode() const { return m_shadingMode; }

		inline void SetVisualizationMode(VisualizationMode visMode) { m_visualizationMode = visMode; }
		inline VisualizationMode GetVisualizationMode() const { return m_visualizationMode; }

		RefPtr<RHI::Image> GetFinalImage();
		RefPtr<RHI::Image> GetObjectIDImage();

		// #TODO_Ivar: TEMP, Should not be public!
		void Invalidate();

		void Enable();

		const uint64_t GetFrameTotalGPUAllocationSize() const;

	private:
		void OnRender(Ref<Camera> camera);

		void BuildMeshPass(RenderGraph::Builder& builder, RenderGraphBlackboard& blackboard);
		void SetupMeshPassConstants(RenderContext& context, const RenderGraphBlackboard& blackboard);

		void SetupFrameData(RenderGraphBlackboard& blackboard, Ref<Camera> camera);

		///// Passes //////
		void UploadUniformBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera);
		void UploadLightBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddExternalResources(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddMainCullingPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddPreDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddObjectIDPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddVisibilityBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddClearGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddGenerateMaterialCountsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddCollectMaterialPixelsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddGenerateMaterialIndirectArgsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void RenderMaterials(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddGenerateGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, const uint32_t materialId);
		void AddSkyboxPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddShadingPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddFXAAPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphImageHandle srcImage);

		void AddFinalCopyPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphImageHandle srcImage);

		void AddVisualizeSDFPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphImageHandle dstImage);
		void AddVisualizeBricksPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphImageHandle dstImage);

		void CreateMainRenderTarget(const uint32_t width, const uint32_t height);

		bool m_enabled = false;

		RefPtr<RHI::Image> m_outputImage;
		RefPtr<RHI::Image> m_objectIDImage;
		RefPtr<RHI::Image> m_previousDepthImage;
		RefPtr<RHI::Image> m_previousColorImage;

		Ref<Mesh> m_skyboxMesh;

		bool m_shouldResize = false;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		uint32_t m_resizeWidth = 1280;
		uint32_t m_resizeHeight = 1280;

		ShadingMode m_shadingMode = ShadingMode::Shaded;
		VisualizationMode m_visualizationMode = VisualizationMode::None;
		PreviousFrameData m_previousFrameData;

		RHI::CommandBufferSet m_commandBufferSet;

		std::atomic<uint64_t> m_frameTotalGPUAllocation;

		///// TEMP /////
		VisibilityVisualization m_visibilityVisualization = VisibilityVisualization::TriangleID;
		////////////////

		Ref<Scene> m_scene;
		SceneEnvironment m_sceneEnvironment;
	};
}
