#pragma once

#include <CoreUtilities/Pointers/RefPtr.h>
#include <CoreUtilities/Pointers/WeakPtr.h>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
	}

	class GPUReadbackBuffer
	{
	public:
		GPUReadbackBuffer(size_t size);

		VT_NODISCARD VT_INLINE RefPtr<RHI::StorageBuffer> GetBuffer() const { return m_buffer; }
		VT_NODISCARD VT_INLINE bool IsReady() const { return m_isReady.load(); }

	private:
		friend class RenderGraph;

		std::atomic_bool m_isReady = false;
		RefPtr<RHI::StorageBuffer> m_buffer;
	};
}
