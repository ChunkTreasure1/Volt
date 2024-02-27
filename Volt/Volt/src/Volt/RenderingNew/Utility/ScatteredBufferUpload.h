#pragma once

#include "Volt/Core/Base.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphUtils.h"

#include "Volt/RenderingNew/Shader/ShaderMap.h"

#include "Volt/Math/Math.h"

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
	}

	class RenderGraph;

	template<typename T>
	concept IsTrivial = std::is_trivially_copyable<T>::value;

	template<IsTrivial T>
	class ScatteredBufferUpload
	{
	public:
		ScatteredBufferUpload(Ref<RHI::StorageBuffer> dstBuffer);

		T& AddUploadItem(size_t bufferIndex);
		void Upload(RenderGraph& renderGraph);

	private:
		std::vector<T> m_data;
		std::vector<uint32_t> m_dataIndices;

		Ref<RHI::StorageBuffer> m_dstBuffer;
	};

	template<IsTrivial T>
	inline ScatteredBufferUpload<T>::ScatteredBufferUpload(Ref<RHI::StorageBuffer> dstBuffer)
		: m_dstBuffer(dstBuffer)
	{
	}

	template<IsTrivial T>
	inline T& ScatteredBufferUpload<T>::AddUploadItem(size_t bufferIndex)
	{
		m_dataIndices.push_back(static_cast<uint32_t>(bufferIndex));
		return m_data.emplace_back();
	}

	template<IsTrivial T>
	inline void ScatteredBufferUpload<T>::Upload(RenderGraph& renderGraph)
	{
		struct ResourceHandles
		{
			RenderGraphResourceHandle srcBuffer;
			RenderGraphResourceHandle dstBuffer;
			RenderGraphResourceHandle indicesBuffer;
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
			data.dstBuffer = renderGraph.AddExternalBuffer(m_dstBuffer, false);
		}

		renderGraph.AddMappedBufferUpload(data.srcBuffer, m_data.data(), sizeof(T) * m_data.size(), "Upload Src Data");
		renderGraph.AddMappedBufferUpload(data.indicesBuffer, m_dataIndices.data(), sizeof(T) * m_dataIndices.size(), "Upload Src Indices");

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
			const uint32_t groupSize = Math::DivideRoundUp(static_cast<uint32_t>(sizeof(T) / 4u * m_data.size()), 64u);

			auto pipeline = ShaderMap::GetComputePipeline("ScatterUpload");

			context.BindPipeline(pipeline);
			context.SetConstant("dstBuffer", resources.GetBuffer(data.dstBuffer));
			context.SetConstant("srcBuffer", resources.GetBuffer(data.srcBuffer));
			context.SetConstant("scatterIndices", resources.GetBuffer(data.indicesBuffer));
			context.SetConstant("typeSize", static_cast<uint32_t>(sizeof(T)));
			context.SetConstant("copyCount", static_cast<uint32_t>(m_data.size()));
			context.Dispatch(groupSize, 1, 1);
		});
	}
}
