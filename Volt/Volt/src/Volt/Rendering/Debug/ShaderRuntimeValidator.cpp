#include "vtpch.h"

#ifndef VT_DIST
#include "ShaderRuntimeValidator.h"

#include "Volt/Core/Application.h"
#include "Volt/Core/Window.h"

#include <RHIModule/Buffers/StorageBuffer.h>
#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Graphics/Swapchain.h>

namespace Volt
{
	constexpr uint32_t MAX_ERROR_COUNT = 1000;

	// These types MUST match types in ShaderRuntimeValidator.hlsli & Resources.hlsli
	enum class RuntimeErrorType : uint32_t
	{
		Invalid = 0,
		InvalidResourceHandleType = 1
	};

	enum class ValidationResourceType : uint32_t
	{
		Invalid = 0,
		Buffer = 1,
		RWBuffer = 2,
		UniformBuffer = 3,
		Texture1D = 4,
		Texture2D = 5,
		Texture3D = 6,
		TextureCube = 7,
		RWTexture1D = 8,
		RWTexture2D = 9,
		RWTexture3D = 10,
		RWTexture2DArray = 11,
		SamplerState = 12,
	};

	inline std::string ResourceTypeToString(ValidationResourceType resourceType)
	{
		switch (resourceType)
		{
			case ValidationResourceType::Invalid: return "Invalid";
			case ValidationResourceType::Buffer: return "Buffer";
			case ValidationResourceType::RWBuffer: return "Read-Write Buffer";
			case ValidationResourceType::UniformBuffer: return "Uniform Buffer";
			case ValidationResourceType::Texture1D: return "Texture 1D";
			case ValidationResourceType::Texture2D: return "Texture 2D";
			case ValidationResourceType::Texture3D: return "Texture 3D";
			case ValidationResourceType::TextureCube: return "Texture Cube";
			case ValidationResourceType::RWTexture1D: return "Read-Write Texture 1D";
			case ValidationResourceType::RWTexture2D: return "Read-Write Texture 2D";
			case ValidationResourceType::RWTexture3D: return "Read-Write Texture 3D";
			case ValidationResourceType::RWTexture2DArray: return "Read-Write Texture 2D Array";
			case ValidationResourceType::SamplerState: return "Sampler State";
		}

		return "Unknown";
	}

	struct RuntimeValidationError
	{
		RuntimeErrorType errorType;
		uint32_t userData0;
		uint32_t userData1;
		uint32_t userData2;
	};

	ShaderRuntimeValidator::ShaderRuntimeValidator()
	{
		m_errorBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(static_cast<uint32_t>(MAX_ERROR_COUNT), sizeof(uint32_t), "Error Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferSrc, RHI::MemoryUsage::GPU);
		m_commandBuffer = RHI::CommandBuffer::Create(2);
	}

	ShaderRuntimeValidator::~ShaderRuntimeValidator()
	{
		m_errorBuffer = nullptr;
	}

	void ShaderRuntimeValidator::Update()
	{
		VT_PROFILE_FUNCTION();

		if (m_stagingBuffer)
		{
			Vector<uint8_t> bufferData{};
			bufferData.resize(m_stagingBuffer->GetResource()->GetByteSize());

			uint8_t* stagingPtr = m_stagingBuffer->GetResource()->Map<uint8_t>();
			memcpy_s(bufferData.data(), bufferData.size(), stagingPtr, m_stagingBuffer->GetResource()->GetByteSize());
			m_stagingBuffer->GetResource()->Unmap();

			ProcessValidationBuffer(bufferData);
		}
		else
		{
			m_stagingBuffer = BindlessResource<RHI::StorageBuffer>::CreateScope(MAX_ERROR_COUNT, sizeof(uint32_t), "Error Staging Buffer", RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);
		}

		m_commandBuffer->Begin();
		m_commandBuffer->CopyBufferRegion(m_errorBuffer->GetResource()->GetAllocation(), 0, m_stagingBuffer->GetResource()->GetAllocation(), 0, m_errorBuffer->GetResource()->GetByteSize());
		m_commandBuffer->ClearBuffer(m_errorBuffer->GetResource(), 0);
		m_commandBuffer->End();
		m_commandBuffer->Execute();
	}

	const ResourceHandle ShaderRuntimeValidator::GetCurrentErrorBufferHandle() const
	{
		return m_errorBuffer->GetResourceHandle();
	}

	void ShaderRuntimeValidator::ProcessValidationBuffer(const Vector<uint8_t>& dataBuffer)
	{
		m_validationErrors.clear();

		constexpr size_t baseOffset = sizeof(uint32_t);
		constexpr size_t stride = sizeof(RuntimeValidationError);

		const uint32_t errorCount = *(uint32_t*)dataBuffer.data();
		
		if (errorCount == 0)
		{
			return;
		}

		m_validationErrors.reserve(errorCount);

		for (size_t i = baseOffset; i < errorCount * stride + baseOffset; i += stride)
		{
			const RuntimeValidationError& error = *(RuntimeValidationError*)(&dataBuffer.at(i));

			auto& newErrorString = m_validationErrors.emplace_back();
			newErrorString = "[Shader Runtime Error]: ";

			switch (error.errorType)
			{
				case RuntimeErrorType::InvalidResourceHandleType:
				{
					newErrorString += "Invalid Resource Handle Type: ";
					newErrorString += std::format("Trying to access resource of type {} as a {}!", ResourceTypeToString(static_cast<ValidationResourceType>(error.userData0)), ResourceTypeToString(static_cast<ValidationResourceType>(error.userData1)));

					break;
				}
			}
		}
	}
}
#endif
