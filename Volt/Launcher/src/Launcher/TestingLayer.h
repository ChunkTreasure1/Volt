#pragma once

#include <Volt/Core/Layer/Layer.h>
#include <Volt/Events/ApplicationEvent.h>

namespace Volt::RHI
{
	class CommandBuffer;
	class Shader;
	class RenderPipeline;

	class Image2D;

	class VertexBuffer;
	class IndexBuffer;

	class DescriptorTable;
	class UniformBuffer;

	class StorageBuffer;
}

namespace Volt
{
	class Mesh;
}

class TestingLayer : public Volt::Layer
{
public:
	TestingLayer() = default;
	~TestingLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Volt::Event& e) override;

private:
	bool OnRenderEvent(Volt::AppRenderEvent& e);
	bool OnUpdateImGui(Volt::AppImGuiUpdateEvent& e);

	Ref<Volt::RHI::Shader> m_shader;
	Ref<Volt::RHI::CommandBuffer> m_commandBuffer;
	Ref<Volt::RHI::RenderPipeline> m_renderPipeline;

	Ref<Volt::RHI::Image2D> m_renderTarget;
	Ref<Volt::RHI::DescriptorTable> m_descriptorTable;
	Ref<Volt::RHI::UniformBuffer> m_constantBuffer;

	Ref<Volt::RHI::StorageBuffer> m_indirectCommandsBuffer;
	Ref<Volt::RHI::StorageBuffer> m_transformsBuffer;

	Ref<Volt::Mesh> m_cubeMesh;
	Ref<Volt::Mesh> m_sphereMesh;

	uint32_t m_currentMesh = 0;
};
