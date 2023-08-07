#pragma once

struct ID3D12Fence;

namespace Volt::RHI
{
	class D3D12Fence
	{
	public:
		void Create(QueueType type);
		ID3D12Fence* Get();
		size_t& Value();
		size_t StartValue();
		void Wait();
		void Signal();
		void Increment() { m_fenceValue++; }
		void SignalStart();
	private:
		ID3D12Fence* m_fence;
		size_t m_fenceValue;
		size_t m_fenceStartValue;
		void* m_windowsFenceHandle;
		QueueType m_type;
	};
}
