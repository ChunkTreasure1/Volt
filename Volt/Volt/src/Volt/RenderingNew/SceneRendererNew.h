#pragma once

#include "Volt/Core/Base.h"

#include "Volt/RenderingNew/RendererCommon.h"
#include "Volt/RenderingNew/SceneRendererStructs.h"
#include "Volt/RenderingNew/Resources/GlobalResource.h"
#include "Volt/RenderingNew/RendererStructs.h"

// #TODO_Ivar: Maybe remove from here
#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"

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

		const uint64_t GetFrameTotalGPUAllocationSize() const;

	private:
		void OnRender(Ref<Camera> camera);

		void BuildMeshPass(RenderGraph::Builder& builder, RenderGraphBlackboard& blackboard);
		void RenderMeshes(RenderContext& context, const RenderGraphPassResources& resources, const RenderGraphBlackboard blackboard);

		///// Passes //////
		void UploadUniformBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera);
		void UploadLightBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void SetupDrawContext(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddExternalResources(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		
		RenderGraphResourceHandle AddGenerateIndirectArgsPass(RenderGraph& renderGraph, RenderGraphResourceHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName);
		RenderGraphResourceHandle AddGenerateIndirectArgsPassWrapped(RenderGraph& renderGraph, RenderGraphResourceHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName);

		void AddCullObjectsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera);
		void AddCullMeshletsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera);
		void AddCullPrimitivesPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera);

		void AddStatsReadbackPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddPreDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddVisibilityBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddGenerateMaterialCountsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddCollectMaterialPixelsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		void AddGenerateMaterialIndirectArgsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddGenerateGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, bool first, const uint32_t materialId);

		void AddShadingPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		void AddTestPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		
		Ref<RHI::Image2D> m_outputImage;

		bool m_shouldResize = false;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		uint32_t m_resizeWidth = 1280;
		uint32_t m_resizeHeight = 1280;

		Ref<RHI::CommandBuffer> m_commandBuffer;
		uint64_t m_frameIndex = 0;

		std::atomic<uint64_t> m_frameTotalGPUAllocation;

		///// TEMP /////
		VisibilityVisualization m_visibilityVisualization = VisibilityVisualization::TriangleID;
		////////////////

		Ref<Scene> m_scene;
		SceneEnvironment m_sceneEnvironment;
	};
}
