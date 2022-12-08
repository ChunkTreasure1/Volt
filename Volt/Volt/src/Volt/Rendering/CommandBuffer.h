#pragma once

#include "Volt/Rendering/RendererInfo.h"

#include <vector>
#include <functional>

namespace Volt
{
	class CommandBuffer
	{
	public:
		void Clear();
		void Submit(const std::function<void()>&& func);
		void Submit(const RenderCommand& cmd);

		inline static Ref<CommandBuffer> Create() { return CreateRef<CommandBuffer>(); }

	private:

		std::vector<std::function<void()>> myCommands;
		std::vector<RenderCommand> myRenderCommands;
	};
}