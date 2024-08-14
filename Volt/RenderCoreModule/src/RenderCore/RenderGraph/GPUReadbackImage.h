#pragma once

#include <CoreUtilities/Pointers/RefPtr.h>

namespace Volt
{
	namespace RHI
	{
		class Image;
	}

	struct RenderGraphImageDesc;

	class GPUReadbackImage
	{
	public:
		GPUReadbackImage(const RenderGraphImageDesc& desc);

		VT_NODISCARD VT_INLINE RefPtr<RHI::Image> GetImage() const { return m_image; }
		VT_NODISCARD VT_INLINE bool IsReady() const { return m_isReady.load(); }

	private:
		friend class RenderGraph;

		std::atomic_bool m_isReady = false;
		RefPtr<RHI::Image> m_image;
	};
}
