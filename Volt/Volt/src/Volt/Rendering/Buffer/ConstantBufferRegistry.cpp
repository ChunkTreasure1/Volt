#include "vtpch.h"
#include "ConstantBufferRegistry.h"

namespace Volt
{
	void ConstantBufferRegistry::Initialize()
	{
	}

	void ConstantBufferRegistry::Shutdown()
	{
		myRegistry.clear();
	}

	void ConstantBufferRegistry::Register(uint32_t aBinding, Ref<ConstantBuffer> aConstantBuffer)
	{
		auto it = myRegistry.find(aBinding);
		if (it != myRegistry.end())
		{
			VT_CORE_WARN("Constant buffer already registered to that binding!");
			return;
		}

		myRegistry.emplace(aBinding, aConstantBuffer);
	}

	Ref<ConstantBuffer> ConstantBufferRegistry::Get(uint32_t aBinding)
	{
		auto it = myRegistry.find(aBinding);
		if (it == myRegistry.end())
		{
			VT_CORE_WARN("Constant buffer not registered to binding {0}!", aBinding);
			return nullptr;
		}

		return it->second;
	}
}