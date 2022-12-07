#pragma once

#include "Volt/Core/Base.h"

#include <unordered_map>

namespace Volt
{
	class ConstantBuffer;
	class ConstantBufferRegistry
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Register(uint32_t aBinding, Ref<ConstantBuffer> aConstantBuffer);
		static Ref<ConstantBuffer> Get(uint32_t aBinding);

	private:
		ConstantBufferRegistry() = delete;

		inline static std::unordered_map<uint32_t, Ref<ConstantBuffer>> myRegistry;
	};
}