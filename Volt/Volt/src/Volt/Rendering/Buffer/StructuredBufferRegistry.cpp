#include "vtpch.h"
#include "StructuredBufferRegistry.h"

#include "Volt/Log/Log.h"

namespace Volt
{
	void StructuredBufferRegistry::Initialize()
	{}

	void StructuredBufferRegistry::Shutdown()
	{
		myRegistry.clear();
	}

	void StructuredBufferRegistry::Register(uint32_t binding, Ref<StructuredBuffer> buffer)
	{
		auto it = myRegistry.find(binding);
		if (it != myRegistry.end())
		{
			VT_CORE_WARN("Structured buffer already registered to that binding!");
			return;
		}

		myRegistry.emplace(binding, buffer);
	}

	Ref<StructuredBuffer> StructuredBufferRegistry::Get(uint32_t binding)
	{
		auto it = myRegistry.find(binding);
		if (it == myRegistry.end())
		{
			VT_CORE_WARN("Structured buffer not registered to binding {0}!", binding);
			return nullptr;
		}

		return it->second;
	}
}