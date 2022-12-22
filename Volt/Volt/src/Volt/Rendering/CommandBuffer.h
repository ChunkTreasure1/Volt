#pragma once

#include "Volt/Rendering/RendererInfo.h"

#include <vector>
#include <functional>

struct ID3D11CommandList;

namespace Volt
{
	class CommandBuffer
	{
	public:
		void Clear();
		void Execute();

		void Submit(const std::function<void()>&& func);
		void Submit(const SubmitCommand& cmd);

		inline std::vector<SubmitCommand>& GetRenderCommands() { return myRenderCommands; }
		inline std::vector<InstancedRenderCommand>& GetCurrentRenderCommands() { return myCurrentRenderCommands; }
		inline static Ref<CommandBuffer> Create() { return CreateRef<CommandBuffer>(); }

		ID3D11CommandList*& GetAndReleaseCommandList();

	private:

		std::vector<std::function<void()>> myCommands;
		std::vector<SubmitCommand> myRenderCommands;
		std::vector<InstancedRenderCommand> myCurrentRenderCommands;
		ID3D11CommandList* myCommandList = nullptr;
	};
}