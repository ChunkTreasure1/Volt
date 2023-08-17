#pragma once

#include "Volt/RenderingNew/RenderObject.h"

namespace Volt
{
	namespace RHI
	{
		class Image2D;
		class CommandBuffer;
		class Shader;
		class RenderPipeline;
		class ComputePipeline;

		class ConstantBuffer;
		class StorageBuffer;

		class DescriptorTable;
	}

	class Camera;
	class Scene;

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

		Ref<RHI::Image2D> m_outputImage;

		bool m_shouldResize = false;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		Ref<RHI::CommandBuffer> m_commandBuffer;

		Ref<RHI::Shader> m_shader;
		Ref<RHI::Shader> m_indirectSetupShader;
		Ref<RHI::Shader> m_clearIndirectCountsShader;

		Ref<RHI::RenderPipeline> m_renderPipeline;
		Ref<RHI::ComputePipeline> m_indirectSetupPipeline;
		Ref<RHI::ComputePipeline> m_clearIndirectCountsPipeline;

		Ref<RHI::ConstantBuffer> m_constantBuffer;

		Ref<RHI::StorageBuffer> m_indirectCommandsBuffer;
		Ref<RHI::StorageBuffer> m_indirectCountsBuffer;
		
		Ref<RHI::StorageBuffer> m_drawToObjectIDBuffer;
		Ref<RHI::StorageBuffer> m_indirectDrawDataBuffer;

		Ref<RHI::DescriptorTable> m_indirectSetupDescriptorTable;
		Ref<RHI::DescriptorTable> m_indirectCountDescriptorTable;
		Ref<RHI::DescriptorTable> m_descriptorTable;

		std::vector<RenderObject> m_activeRenderObjects;
		uint32_t m_currentActiveCommandCount = 0;

		Weak<Scene> m_scene;
	};
}
