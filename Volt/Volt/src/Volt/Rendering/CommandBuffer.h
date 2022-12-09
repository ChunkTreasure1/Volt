#pragma once

#include "Volt/Rendering/RendererInfo.h"

#include <vector>
#include <functional>
#include <d3d11.h>

namespace Volt
{
	class CommandBuffer
	{
	public:
		void Clear();
		void Execute();

		void Submit(const std::function<void()>&& func);
		void Submit(const RenderCommand& cmd);

		inline std::vector<RenderCommand>& GetRenderCommands() { return myRenderCommands; }
		inline std::vector<InstancedRenderCommand>& GetCurrentRenderCommands() { return myCurrentRenderCommands; }

		inline static Ref<CommandBuffer> Create() { return CreateRef<CommandBuffer>(); }

	private:

		std::vector<std::function<void()>> myCommands;
		std::vector<RenderCommand> myRenderCommands;
		std::vector<InstancedRenderCommand> myCurrentRenderCommands;
	};
}