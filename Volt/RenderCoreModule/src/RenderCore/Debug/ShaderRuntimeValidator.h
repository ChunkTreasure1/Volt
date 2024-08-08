#pragma once

#include "RenderCore/Config.h"

#ifndef VT_DIST
#include <RHIModule/Descriptors/ResourceHandle.h>

#include <RHIModule/Buffers/StorageBuffer.h>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
		class CommandBuffer;
	}

	class VTRC_API ShaderRuntimeValidator
	{
	public:
		ShaderRuntimeValidator();
		~ShaderRuntimeValidator();

		void Update();
		const ResourceHandle GetCurrentErrorBufferHandle() const;
		const Vector<std::string>& GetValidationErrors() const { return m_validationErrors; }

		VT_INLINE static ShaderRuntimeValidator& Get() { return *s_instance; }

	private:
		inline static ShaderRuntimeValidator* s_instance = nullptr;

		void ProcessValidationBuffer(const Vector<uint8_t>& dataBuffer);

		RefPtr<RHI::StorageBuffer> m_errorBuffer;
		RefPtr<RHI::StorageBuffer> m_stagingBuffer;

		ResourceHandle m_errorBufferHandle;
		ResourceHandle m_stagingBufferHandle;

		RefPtr<RHI::CommandBuffer> m_commandBuffer;

		Vector<std::string> m_validationErrors;
	};
}

#endif
