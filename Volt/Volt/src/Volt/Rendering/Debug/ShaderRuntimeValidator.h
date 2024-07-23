#pragma once

#ifndef VT_DIST
#include "Volt/Rendering/Resources/BindlessResource.h"

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
		class CommandBuffer;
	}

	class ShaderRuntimeValidator
	{
	public:
		ShaderRuntimeValidator();
		~ShaderRuntimeValidator();

		void Update();
		const ResourceHandle GetCurrentErrorBufferHandle() const;
		const Vector<std::string>& GetValidationErrors() const { return m_validationErrors; }

	private:
		void ProcessValidationBuffer(const Vector<uint8_t>& dataBuffer);

		BindlessResourceScope<RHI::StorageBuffer> m_errorBuffer;
		BindlessResourceScope<RHI::StorageBuffer> m_stagingBuffer;

		RefPtr<RHI::CommandBuffer> m_commandBuffer;

		Vector<std::string> m_validationErrors;
	};
}

#endif
