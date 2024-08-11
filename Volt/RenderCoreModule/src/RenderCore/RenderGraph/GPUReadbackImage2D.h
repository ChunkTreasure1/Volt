#pragma once

#include <CoreUtilities/Pointers/RefPtr.h>

namespace Volt
{
	namespace RHI
	{
		class Image2D;
	}

	struct RenderGraphImageDesc;

	class GPUReadbackImage2D
	{
	public:
		GPUReadbackImage2D(const RenderGraphImageDesc& desc);

		VT_NODISCARD VT_INLINE RefPtr<RHI::Image2D> GetImage() const { return m_image; }
		VT_NODISCARD VT_INLINE bool IsReady() const { return m_isReady.load(); }

	private:
		friend class RenderGraph;

		std::atomic_bool m_isReady = false;
		RefPtr<RHI::Image2D> m_image;
	};
}
