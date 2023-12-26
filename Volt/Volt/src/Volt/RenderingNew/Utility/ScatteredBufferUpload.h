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
	concept IsTrivial = std::is_trivial<T>::value;

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
		barrier.resource = m_dstBuffer;
		barrier.oldState = RHI::ResourceState::UnorderedAccess | RHI::ResourceState::PixelShaderRead | RHI::ResourceState::NonPixelShaderRead;
		barrier.newState = RHI::ResourceState::TransferDst;

		commandBuffer->ResourceBarrier({ barrier });

		constexpr size_t stride = sizeof(T);

		for (const auto& element : m_data)
		{
			const size_t offset = stride * element.bufferIndex;
			commandBuffer->UpdateBuffer(m_dstBuffer, offset, stride, element.data);
		}

		std::swap(barrier.oldState, barrier.newState);
		commandBuffer->ResourceBarrier({ barrier });
		commandBuffer->End();
	}
}
