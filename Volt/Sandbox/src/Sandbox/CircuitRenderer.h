#pragma once

#include <Volt/Core/WindowHandle.h>
#include <CoreUtilities/Core.h>

namespace Circuit
{
	class CircuitWindow;
}

struct CircuitOutputData;
namespace Volt
{
	namespace RHI
	{
		class Image2D;
		class CommandBuffer;
	}

	class RenderGraph;
	class RenderGraphBlackboard;
	class Window;
}
class CircuitRenderer
{
public:
	CircuitRenderer(Volt::WindowHandle& windowHandle);
	~CircuitRenderer() = default;

	void OnRender();

private:
	CircuitOutputData& AddCircuitPrimitivesPass(Volt::RenderGraph& renderGraph, Volt::RenderGraphBlackboard& blackboard);

	Circuit::CircuitWindow& m_targetCircuitWindow;
	Volt::Window& m_targetWindow;

	uint32_t m_width;
	uint32_t m_height;

	RefPtr<Volt::RHI::Image2D> m_outputImage;
	RefPtr<Volt::RHI::CommandBuffer> m_commandBuffer;

	std::atomic<uint64_t> m_frameTotalGPUAllocation;
};
