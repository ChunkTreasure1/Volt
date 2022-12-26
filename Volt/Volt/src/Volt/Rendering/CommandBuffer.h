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
		inline static Ref<CommandBuffer> Create() { return CreateRef<CommandBuffer>(); }

		ID3D11CommandList*& GetAndReleaseCommandList();

	private:
		std::vector<std::function<void()>> myCommands;
		ID3D11CommandList* myCommandList = nullptr;
	};
}