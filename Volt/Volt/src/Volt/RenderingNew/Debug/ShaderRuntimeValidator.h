#pragma once

#ifndef VT_DIST
#include "Volt/RenderingNew/Resources/GlobalResource.h"

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
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

		Scope<GlobalResource<RHI::StorageBuffer>> m_errorBuffer;
		Scope<GlobalResource<RHI::StorageBuffer>> m_stagingBuffer;

		std::vector<std::string> m_validationErrors;
	};
}

#endif
