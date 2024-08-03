#pragma once

#include "Volt/Rendering/RenderGraph/RenderGraph.h"


#include <atomic>

namespace Volt
{
	template<typename T>
	class StagedBufferUpload
	{
	public:
		StagedBufferUpload(const uint32_t uploadCount);

		T& AddUploadItem();
		void UploadTo(RenderGraph& renderGraph, RenderGraphBufferHandle dstBuffer);

	private:
		Vector<T> m_data;
		std::atomic<uint32_t> m_currentIndex = 0;
	};

	template<typename T>
	inline StagedBufferUpload<T>::StagedBufferUpload(const uint32_t uploadCount)
	{
		m_data.resize(uploadCount);
	}
	
	template<typename T>
	inline T& StagedBufferUpload<T>::AddUploadItem()
	{
		const uint32_t index = m_currentIndex++;
		return m_data[index];
	}

	template<typename T>
	inline void StagedBufferUpload<T>::UploadTo(RenderGraph& renderGraph, RenderGraphBufferHandle dstBuffer)
	{
		if (m_currentIndex == 0)
		{
			return;
		}

		renderGraph.AddStagedBufferUpload(dstBuffer, m_data.data(), m_data.size() * sizeof(T), "Staged Buffer Upload");
	}
}
