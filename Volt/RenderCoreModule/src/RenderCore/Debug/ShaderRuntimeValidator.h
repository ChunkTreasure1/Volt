#pragma once

#include "RenderCore/Config.h"

#ifdef VT_ENABLE_SHADER_RUNTIME_VALIDATION

#include "RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h"

#include <RHIModule/Descriptors/ResourceHandle.h>
#include <RHIModule/Buffers/StorageBuffer.h>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
		class CommandBuffer;
	}

	class RenderGraph;

	class VTRC_API ShaderRuntimeValidator
	{
	public:
		ShaderRuntimeValidator();
		~ShaderRuntimeValidator();

		void Allocate(RenderGraph& renderGraph);
		
		void ReadbackErrorBuffer(RenderGraph& renderGraph);
		const RenderGraphBufferHandle GetErrorBufferHandle() const;
		const Vector<std::string>& GetValidationErrors() const { return m_validationErrors; }

	private:
		void ProcessValidationBuffer(const Vector<uint8_t>& dataBuffer);

		RenderGraphBufferHandle m_errorBufferHandle;
		Vector<std::string> m_validationErrors;
	};
}

#endif
