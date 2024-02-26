#pragma once

#include "Volt/Core/Base.h"

#include <VoltRHI/Buffers/CommandBuffer.h>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
	}

	template<typename T>
	concept IsTrivial = std::is_trivially_copyable<T>::value;

	template<IsTrivial T>
	class ScatteredBufferUpload
	{
	public:
		ScatteredBufferUpload(Ref<RHI::StorageBuffer> dstBuffer);

		T& AddUploadItem(size_t bufferIndex);
		void Upload();
		void UploadAndWait();

	private:
		void UploadInternal(Ref<RHI::CommandBuffer> commandBuffer);

		struct DataContainer
		{
			size_t bufferIndex;
			T data;
		};

		std::vector<DataContainer> m_data;
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
		auto& element = m_data.emplace_back(bufferIndex, T{});
		return element.data;
	}

	template<IsTrivial T>
	inline void ScatteredBufferUpload<T>::Upload()
	{
		// #TODO_Ivar: We probably want to use a transfer queue instead
		Ref<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();
		UploadInternal(commandBuffer);
		commandBuffer->Execute();
	}

	template<IsTrivial T>
	inline void ScatteredBufferUpload<T>::UploadAndWait()
	{
		// #TODO_Ivar: We probably want to use a transfer queue instead
		Ref<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();
		UploadInternal(commandBuffer);
		commandBuffer->ExecuteAndWait();
	}

	template<IsTrivial T>
	inline void ScatteredBufferUpload<T>::UploadInternal(Ref<RHI::CommandBuffer> commandBuffer)
	{
		commandBuffer->Begin();

		RHI::ResourceBarrierInfo barrier{};
		barrier.type = RHI::BarrierType::Buffer;
		barrier.bufferBarrier().srcAccess = RHI::BarrierAccess::ShaderRead | RHI::BarrierAccess::ShaderWrite;
		barrier.bufferBarrier().srcStage = RHI::BarrierStage::AllGraphics;
		barrier.bufferBarrier().dstAccess = RHI::BarrierAccess::TransferDestination;
		barrier.bufferBarrier().dstStage = RHI::BarrierStage::Copy;
		barrier.bufferBarrier().resource = m_dstBuffer;
		barrier.bufferBarrier().size = m_dstBuffer->GetByteSize();

		commandBuffer->ResourceBarrier({ barrier });

		constexpr size_t stride = sizeof(T);

		for (const auto& element : m_data)
		{
			const size_t offset = stride * element.bufferIndex;
			commandBuffer->UpdateBuffer(m_dstBuffer, offset, stride, &element.data);
		}

		std::swap(barrier.bufferBarrier().srcAccess, barrier.bufferBarrier().dstAccess);
		std::swap(barrier.bufferBarrier().srcStage, barrier.bufferBarrier().dstStage);

		commandBuffer->ResourceBarrier({ barrier });
		commandBuffer->End();
	}
}
