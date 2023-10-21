#pragma once

#include "Volt/Core/Base.h"

#include "Volt/RenderingNew/SceneRendererStructs.h"

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
		class StorageBufferSet;
		class StorageBuffer;

		class SamplerState;

		class DescriptorTable;
	}

	class Camera;
	class Scene;
	class RenderScene;
	class RenderContext;

	class RenderGraph;
	class RenderGraphBlackboard;

	struct SceneRendererSpecification
	{
		std::string debugName;
		glm::uvec2 initialResolution = { 1280, 720 };
		Ref<Scene> scene;
	};

	class SceneRendererNew
	{
	public:
		SceneRendererNew(const SceneRendererSpecification& specification);
		~SceneRendererNew();

		void OnRenderEditor(Ref<Camera> camera);

		void Resize(const uint32_t width, const uint32_t height);

		Ref<RHI::Image2D> GetFinalImage();

		// #TODO_Ivar: TEMP, Should not be public!
		void Invalidate();
	private:
		void OnRender(Ref<Camera> camera);

		void UpdateBuffers(Ref<Camera> camera);
		void UpdateCameraBuffer(Ref<Camera> camera);
		void UpdateSamplersBuffer();
		void UpdateLightBuffers();

		void UpdateDescriptorTableForMeshRendering(RenderScene& renderScene, RenderContext& renderContext);
		void UpdateDescriptorTableForBindlessRendering(RenderScene& renderScene, RenderContext& renderContext);

		void CreatePipelines();

		///// Passes //////
		void AddExternalResources(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddSetupIndirectPasses(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddPreDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddVisibilityBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddVisibilityVisualizationPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddGenerateMaterialCountsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddCollectMaterialPixelsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddGenerateMaterialIndirectArgsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddGenerateGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		///////////////////

		Ref<RHI::Image2D> m_outputImage;

		bool m_shouldResize = false;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		uint32_t m_resizeWidth = 1280;
		uint32_t m_resizeHeight = 1280;

		Ref<RHI::CommandBuffer> m_commandBuffer;

		Ref<RHI::RenderPipeline> m_renderPipeline;

		Ref<RHI::UniformBufferSet> m_constantBufferSet;
		Ref<RHI::StorageBufferSet> m_storageBufferSet;

		Ref<RHI::StorageBuffer> m_indirectCommandsBuffer;
		Ref<RHI::StorageBuffer> m_indirectCountsBuffer;
		
		Ref<RHI::StorageBuffer> m_drawToInstanceOffsetBuffer;
		Ref<RHI::StorageBuffer> m_instanceOffsetToObjectIDBuffer;
		Ref<RHI::StorageBuffer> m_indirectDrawDataBuffer;
		Ref<RHI::StorageBuffer> m_materialsBuffer;

		Ref<RHI::SamplerState> m_samplerState;

		uint32_t m_currentActiveCommandCount = 0;

		///// TEMP /////
		VisibilityVisualization m_visibilityVisualization = VisibilityVisualization::TriangleID;
		////////////////

		///// Pipelines /////
		Ref<RHI::ComputePipeline> m_indirectSetupPipeline;
		Ref<RHI::ComputePipeline> m_clearIndirectCountsPipeline;
		Ref<RHI::ComputePipeline> m_visibilityVisualizationPipeline;
		Ref<RHI::ComputePipeline> m_generateMaterialCountPipeline;
		Ref<RHI::ComputePipeline> m_collectMaterialPixelsPipeline;
		Ref<RHI::ComputePipeline> m_generateMaterialIndirectArgsPipeline;
		Ref<RHI::ComputePipeline> m_generateGBufferPipeline;

		Ref<RHI::ComputePipeline> m_prefixSumPipeline;

		Ref<RHI::RenderPipeline> m_preDepthPipeline;
		Ref<RHI::RenderPipeline> m_visibilityPipeline;
		/////////////////////

		Ref<Scene> m_scene;
	};
}
