#pragma once

#include "Volt/Core/Base.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"

#include "Volt/Rendering/Shader/ShaderMap.h"

#include "Volt/Math/Math.h"

#include <atomic>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
	}

	class RenderGraph;

	template<typename T>
	concept IsTrivial = std::is_trivially_copyable<T>::value && sizeof(T) % 4 == 0;

	template<IsTrivial T>
	class ScatteredBufferUpload
	{
	public:
		ScatteredBufferUpload(const size_t uploadCount);

		T& AddUploadItem(size_t bufferIndex);
		void UploadTo(RenderGraph& renderGraph, Ref<RHI::StorageBuffer> dstBuffer);
		void UploadTo(RenderGraph& renderGraph, const GlobalResource<RHI::StorageBuffer>& dstBuffer);

	private:
		void UploadToInternal(RenderGraph& renderGraph, Ref<RHI::StorageBuffer> dstBuffer, bool trackGlobalResource);

		std::vector<T> m_data;
		std::vector<uint32_t> m_dataIndices;
		std::atomic<uint32_t> m_currentIndex = 0;
	};

	template<IsTrivial T>
	inline ScatteredBufferUpload<T>::ScatteredBufferUpload(const size_t uploadCount)
	{
		m_data.resize(uploadCount);
		m_dataIndices.resize(uploadCount);
	}

	template<IsTrivial T>
	inline T& ScatteredBufferUpload<T>::AddUploadItem(size_t bufferIndex)
	{
		const uint32_t index = m_currentIndex++;

		m_dataIndices[index] = static_cast<uint32_t>(bufferIndex);
		return m_data[index];
	}

	template<IsTrivial T>
	inline void ScatteredBufferUpload<T>::UploadTo(RenderGraph& renderGraph, Ref<RHI::StorageBuffer> dstBuffer)
	{
		UploadToInternal(renderGraph, dstBuffer, true);
	}

	template<IsTrivial T>
	inline void ScatteredBufferUpload<T>::UploadTo(RenderGraph& renderGraph, const GlobalResource<RHI::StorageBuffer>& dstBuffer)
	{
		UploadToInternal(renderGraph, dstBuffer.GetResource(), false);
	}

	template<IsTrivial T>
	inline void ScatteredBufferUpload<T>::UploadToInternal(RenderGraph& renderGraph, Ref<RHI::StorageBuffer> dstBuffer, bool trackGlobalResource)
	{
		if (m_data.empty())
		{
			return;
		}

		struct ResourceHandles
		{
			RenderGraphResourceHandle srcBuffer;
			RenderGraphResourceHandle dstBuffer;
			RenderGraphResourceHandle indicesBuffer;

			uint32_t dataCount = 0;
		} data;

		{
			const auto desc = RGUtils::CreateBufferDesc<T>(m_data.size(), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Src Data");
			data.srcBuffer = renderGraph.CreateBuffer(desc);
		}

		{
			const auto desc = RGUtils::CreateBufferDesc<uint32_t>(m_data.size(), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Scatter Indices");
			data.indicesBuffer = renderGraph.CreateBuffer(desc);
		}

		{
			data.dstBuffer = renderGraph.AddExternalBuffer(dstBuffer, trackGlobalResource);
			data.dataCount = static_cast<uint32_t>(m_data.size());
		}

		renderGraph.AddMappedBufferUpload(data.srcBuffer, m_data.data(), sizeof(T) * m_data.size(), "Upload Src Data");
		renderGraph.AddMappedBufferUpload(data.indicesBuffer, m_dataIndices.data(), sizeof(uint32_t) * m_dataIndices.size(), "Upload Src Indices");

		renderGraph.AddPass("Scatter Buffer Upload",
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(data.dstBuffer);
			builder.ReadResource(data.srcBuffer);
			builder.ReadResource(data.indicesBuffer);

			builder.SetIsComputePass();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			constexpr uint32_t sizeInUINT = static_cast<uint32_t>(sizeof(T) / sizeof(uint32_t));

			const uint32_t groupSize = Math::DivideRoundUp(static_cast<uint32_t>(sizeInUINT * data.dataCount), 64u);

			auto pipeline = ShaderMap::GetComputePipeline("ScatterUpload");

			context.BindPipeline(pipeline);
			context.SetConstant("dstBuffer"_sh, resources.GetBuffer(data.dstBuffer));
			context.SetConstant("srcBuffer"_sh, resources.GetBuffer(data.srcBuffer));
			context.SetConstant("scatterIndices"_sh, resources.GetBuffer(data.indicesBuffer));
			context.SetConstant("typeSizeInUINT"_sh, static_cast<uint32_t>(sizeInUINT));
			context.SetConstant("copyCount"_sh, data.dataCount);
			context.Dispatch(groupSize, 1, 1);
		});
	}
}
