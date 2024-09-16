#pragma once

#include <RHIModule/Descriptors/ResourceHandle.h>

#include <CoreUtilities/Pointers/WeakPtr.h>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
		class UniformBuffer;
	}

	class SharedRenderContext
	{
	public:
		~SharedRenderContext();

		void BeginContext();
		void EndContext();

		uint8_t* GetRenderGraphConstantsPointer(uint32_t passIndex);
		VT_NODISCARD VT_INLINE WeakPtr<RHI::UniformBuffer> GetRenderGraphConstantsBuffer() const { return m_renderGraphConstantsBuffer; }

		uint8_t* GetPassConstantsPointer(uint32_t passIndex);
		ResourceHandle GetPassConstantsBufferResourceHandle() const;

	private:
		friend class RenderGraph;

		void SetPerPassConstantsBuffer(WeakPtr<RHI::StorageBuffer> constantsBuffer);
		void SetRenderGraphConstantsBuffer(WeakPtr<RHI::UniformBuffer> constantsBuffer);

		bool m_isRenderGraphConstantsMapped = false;
		uint8_t* m_mappedRenderGraphConstantsPointer = nullptr;

		bool m_isPassConstantsMapped = false;
		uint8_t* m_mappedPassConstantsPointer = nullptr;

		WeakPtr<RHI::StorageBuffer> m_passConstantsBuffer;
		WeakPtr<RHI::UniformBuffer> m_renderGraphConstantsBuffer;
	};
}
