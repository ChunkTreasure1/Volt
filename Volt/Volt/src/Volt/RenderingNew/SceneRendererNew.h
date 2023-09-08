#pragma once

#include "Volt/Core/Base.h"

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

	struct SceneRendererSpecification
	{
		std::string debugName;
		glm::uvec2 initialResolution = { 1280, 720 };
		Weak<Scene> scene;
	};

	class SceneRendererNew
	{
	public:
		SceneRendererNew(const SceneRendererSpecification& specification);
		~SceneRendererNew();

		void OnRenderEditor(Ref<Camera> camera);

		void Resize(const uint32_t width, const uint32_t height);

		Ref<RHI::Image2D> GetFinalImage();

	private:
		void OnRender(Ref<Camera> camera);
		void Invalidate();

		void UpdateBuffers(Ref<Camera> camera);
		void UpdateCameraBuffer(Ref<Camera> camera);
		void UpdateLightBuffers();

		void UpdateDescriptorTable(RenderScene& renderScene, RenderContext& renderContext);
		void UpdateDescriptorTable(RenderScene& renderScene);

		void CreatePipelines();

		Ref<RHI::Image2D> m_outputImage;
		Ref<RHI::Image2D> m_depthImage;

		bool m_shouldResize = false;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		Ref<RHI::CommandBuffer> m_commandBuffer;

		Ref<RHI::RenderPipeline> m_renderPipeline;

		Ref<RHI::UniformBufferSet> m_constantBufferSet;
		Ref<RHI::StorageBufferSet> m_storageBufferSet;

		Ref<RHI::StorageBuffer> m_indirectCommandsBuffer;
		Ref<RHI::StorageBuffer> m_indirectCountsBuffer;
		
		Ref<RHI::StorageBuffer> m_drawToInstanceOffsetBuffer;
		Ref<RHI::StorageBuffer> m_instanceOffsetToObjectIDBuffer;
		Ref<RHI::StorageBuffer> m_indirectDrawDataBuffer;

		Ref<RHI::SamplerState> m_samplerState;

		Ref<RHI::DescriptorTable> m_descriptorTable;

		uint32_t m_currentActiveCommandCount = 0;

		// Pipelines
		Ref<RHI::ComputePipeline> m_indirectSetupPipeline;
		Ref<RHI::ComputePipeline> m_clearIndirectCountsPipeline;

		Ref<RHI::RenderPipeline> m_preDepthPipeline;

		Weak<Scene> m_scene;
	};
}
