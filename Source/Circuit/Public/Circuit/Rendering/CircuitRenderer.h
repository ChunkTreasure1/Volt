#pragma once

#include "Circuit/Config.h"

#include <WindowModule/WindowHandle.h>
#include <CoreUtilities/Core.h>
#include <CoreUtilities/Pointers/RefPtr.h>

#include <RHIModule/Buffers/CommandBuffer.h>

namespace Circuit
{
	class CircuitWindow;
}

struct CircuitOutputData;
namespace Volt
{
	namespace RHI
	{
		class Image;
		class CommandBuffer;
	}

	class RenderGraph;
	class RenderGraphBlackboard;
	class Window;
}

namespace Circuit
{
	class CIRCUIT_API CircuitRenderer
	{
	public:
		CircuitRenderer(CircuitWindow& targetCircuitWindow);
		~CircuitRenderer() = default;

		void OnRender();

	private:
		CircuitOutputData& AddCircuitPrimitivesPass(Volt::RenderGraph& renderGraph, Volt::RenderGraphBlackboard& blackboard);

		Circuit::CircuitWindow& m_targetCircuitWindow;
		Volt::Window& m_targetWindow;

		uint32_t m_width;
		uint32_t m_height;

		RefPtr<Volt::RHI::Image> m_outputImage;
		RefPtr<Volt::RHI::CommandBuffer> m_commandBuffer;

		std::atomic<uint64_t> m_frameTotalGPUAllocation;
	};
}
