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
		typedef void(*CommandFn)(void*);

		CommandBuffer();
		~CommandBuffer();

		void Clear();
		void Execute();

		template<typename T>
		void Submit(T&& func);

		inline static Ref<CommandBuffer> Create() { return CreateRef<CommandBuffer>(); }

		ID3D11CommandList*& GetAndReleaseCommandList();

	private:
		void* AllocateCommand(CommandFn func, uint32_t size);

		ID3D11CommandList* myCommandList = nullptr;

		uint8_t* myCommandBufferAllocation = nullptr;
		uint8_t* myCommandBufferPtr = nullptr;
		uint32_t myCommandCount = 0;
	};

	template<typename T>
	inline void CommandBuffer::Submit(T&& func)
	{
		auto renderCmd = [](void* ptr)
		{
			auto funcPtr = (T*)ptr;
			(*funcPtr)();

			// Must be done for strings, vectors and such
			funcPtr->~T();
		};

		auto storageBuffer = AllocateCommand(renderCmd, sizeof(func));
		new (storageBuffer) T(std::forward<T>(func));
	}
}