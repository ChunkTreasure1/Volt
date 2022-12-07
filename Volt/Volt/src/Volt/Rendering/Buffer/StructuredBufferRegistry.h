#pragma once

#include "Volt/Core/Base.h"

#include <unordered_map>

namespace Volt
{
	class StructuredBuffer;
	class StructuredBufferRegistry
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Register(uint32_t binding, Ref<StructuredBuffer> buffer);
		static Ref<StructuredBuffer> Get(uint32_t binding);

	private:
		StructuredBufferRegistry() = delete;

		inline static std::unordered_map<uint32_t, Ref<StructuredBuffer>> myRegistry;
	};
}