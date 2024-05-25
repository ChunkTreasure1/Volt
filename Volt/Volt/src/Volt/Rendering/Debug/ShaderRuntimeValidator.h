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
		const std::vector<std::string>& GetValidationErrors() const { return m_validationErrors; }

	private:
		void ProcessValidationBuffer(const std::vector<uint8_t>& dataBuffer);

		BindlessResourceScope<RHI::StorageBuffer> m_errorBuffer;
		BindlessResourceScope<RHI::StorageBuffer> m_stagingBuffer;

		Ref<RHI::CommandBuffer> m_commandBuffer;

		std::vector<std::string> m_validationErrors;
	};
}

#endif
