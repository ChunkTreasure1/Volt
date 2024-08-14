#pragma once

#include "RenderCore/RenderGraph/RenderGraphCommon.h"
#include "RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h"

#include <RHIModule/Descriptors/ResourceHandle.h>
#include <RHIModule/Core/RHICommon.h>
#include <RHIModule/Shader/Shader.h>
#include <RHIModule/Pipelines/RenderPipeline.h>
#include <RHIModule/Pipelines/ComputePipeline.h>
#include <RHIModule/Buffers/IndexBuffer.h>
#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Synchronization/Fence.h>

#include <CoreUtilities/Profiling/Profiling.h>
#include <CoreUtilities/StringHash.h>	

#include <glm/glm.hpp>
#include <half/half.hpp>

namespace Volt
{
	namespace RHI
	{
		class CommandBuffer;
		class StorageBuffer;
		class VertexBuffer;

		class DescriptorTable;

		class BufferView;
		class ImageView;

		class Image2D;
		class BufferViewSet;
	}

	struct RenderingInfo
	{
		RHI::Rect2D scissor{};
		RHI::Viewport viewport{};
		RHI::RenderingInfo renderingInfo{};
	};

	template<typename T>
	inline static constexpr RHI::ShaderUniformType TryGetTypeFromType()
	{
		RHI::ShaderUniformType resultType{};

		if constexpr (std::is_same_v<T, bool>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Bool;
		}
		else if constexpr (std::is_same_v<T, int16_t>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Short;
		}
		else if constexpr (std::is_same_v<T, uint16_t>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::UShort;
		}
		else if constexpr (std::is_same_v<T, uint32_t>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::UInt;
		}
		else if constexpr (std::is_same_v<T, int32_t>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Int;
		}
		else if constexpr (std::is_same_v<T, int64_t>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Int64;
		}
		else if constexpr (std::is_same_v<T, uint64_t>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::UInt64;
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Double;
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Float;
		}
		else if constexpr (std::is_same_v<T, half_float::half>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Half;
		}

		else if constexpr (std::is_same_v<T, glm::vec2>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Float;
			resultType.vecsize = 2;
		}
		else if constexpr (std::is_same_v<T, glm::vec3>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Float;
			resultType.vecsize = 3;
		}
		else if constexpr (std::is_same_v<T, glm::vec4>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Float;
			resultType.vecsize = 4;
		}
		else if constexpr (std::is_same_v<T, glm::uvec2>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::UInt;
			resultType.vecsize = 2;
		}
		else if constexpr (std::is_same_v<T, glm::uvec3>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::UInt;
			resultType.vecsize = 3;
		}
		else if constexpr (std::is_same_v<T, glm::uvec4>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::UInt;
			resultType.vecsize = 4;
		}
		else if constexpr (std::is_same_v<T, glm::ivec2>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Int;
			resultType.vecsize = 2;
		}
		else if constexpr (std::is_same_v<T, glm::ivec3>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Int;
			resultType.vecsize = 3;
		}
		else if constexpr (std::is_same_v<T, glm::ivec4>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Int;
			resultType.vecsize = 4;
		}

		else if constexpr (std::is_same_v<T, glm::mat4>)
		{
			resultType.baseType = RHI::ShaderUniformBaseType::Float;
			resultType.vecsize = 4;
			resultType.columns = 4;
		}

		return resultType;
	}

	struct RenderGraphPassNodeBase;
	class RenderGraph;

	class VTRC_API RenderContext
	{
	public:
		struct RenderGraphConstants
		{
			ResourceHandle constatsBufferIndex;
			ResourceHandle shaderValidationBuffer;
			uint32_t constantsOffset;
		};

		RenderContext(RefPtr<RHI::CommandBuffer> commandBuffer);
		~RenderContext();

		void BeginMarker(std::string_view markerName, const glm::vec4& markerColor = 1.f);
		void EndMarker();

		void BeginRendering(const RenderingInfo& renderingInfo);
		void EndRendering();

		const RenderingInfo CreateRenderingInfo(const uint32_t width, const uint32_t height, const StackVector<RenderGraphImageHandle, RHI::MAX_ATTACHMENT_COUNT>& attachments);

		void ClearImage(RenderGraphImageHandle handle, const glm::vec4& clearColor);
		void ClearBuffer(RenderGraphBufferHandle handle, uint32_t clearValue);

		void CopyBuffer(RenderGraphBufferHandle src, RenderGraphBufferHandle dst, const size_t size);

		void MappedBufferUpload(RenderGraphBufferHandle buffer, const void* data, const size_t size);
		void PushConstants(const void* data, const uint32_t size);

		void DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ);
		void DispatchMeshTasksIndirect(RenderGraphBufferHandle commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride);
		void DispatchMeshTasksIndirectCount(RenderGraphBufferHandle commandsBuffer, const size_t offset, RenderGraphBufferHandle countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride);

		void Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ);
		void DispatchIndirect(RenderGraphBufferHandle commandsBuffer, const size_t offset);
		
		void DrawIndirectCount(RenderGraphBufferHandle commandsBuffer, const size_t offset, RenderGraphBufferHandle countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride);
		void DrawIndexedIndirect(RenderGraphBufferHandle commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride);
		void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance);
		void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance);

		void BindPipeline(WeakPtr<RHI::RenderPipeline> pipeline);
		void BindPipeline(WeakPtr<RHI::ComputePipeline> pipeline);

		void BindIndexBuffer(RenderGraphBufferHandle indexBuffer);
		void BindIndexBuffer(WeakPtr<RHI::IndexBuffer> indexBuffer);
		void BindVertexBuffers(const StackVector<WeakPtr<RHI::VertexBuffer>, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding);
		void BindVertexBuffers(const StackVector<RenderGraphBufferHandle, RHI::MAX_VERTEX_BUFFER_COUNT>& vertexBuffers, const uint32_t firstBinding);

		template<typename T>
		void SetConstant(const StringHash& name, const T& data);

		template<>
		void SetConstant(const StringHash& name, const ResourceHandle& data);

		void SetConstant(const StringHash& name, const RenderGraphImageHandle& data, const int32_t mip = -1, const int32_t layer = -1);

		template<>
		void SetConstant(const StringHash& name, const RenderGraphBufferHandle& data);

		template<>
		void SetConstant(const StringHash& name, const RenderGraphUniformBufferHandle& data);

		template<typename F>
		void SetConstant(const StringHash& name, const Vector<F>& data);

		template<typename F, size_t COUNT>
		void SetConstant(const StringHash& name, const std::array<F, COUNT>& data);

	private:
		friend class RenderGraph;

		struct BoundPipelineData
		{
			std::unordered_map<StringHash, bool> uniformHasBeenSetMap;
		};

		void BindDescriptorTableIfRequired();

		void SetPerPassConstantsBuffer(WeakPtr<RHI::StorageBuffer> constantsBuffer);
		void SetRenderGraphConstantsBuffer(WeakPtr<RHI::UniformBuffer> constantsBuffer);

		void SetCurrentPass(Weak<RenderGraphPassNodeBase> currentPassNode);
		void SetRenderGraphInstance(RenderGraph* renderGraph);
		void UploadConstantsData();

		void Flush(RefPtr<RHI::Fence> fence);

		void CopyImage(RenderGraphImageHandle src, RenderGraphImageHandle dst, const uint32_t width, const uint32_t height, const uint32_t depth);

		// Validation
		void InitializeCurrentPipelineConstantsValidation();
		void ValidateCurrentPipelineConstants();

		const RHI::ShaderRenderGraphConstantsData& GetRenderGraphConstantsData();

		// Internal state
		bool m_descriptorTableIsBound = false; // This needs to be checked in every call that uses resources

		WeakPtr<RHI::RenderPipeline> m_currentRenderPipeline;
		WeakPtr<RHI::ComputePipeline> m_currentComputePipeline;

		WeakPtr<RHI::StorageBuffer> m_perPassConstantsBuffer;
		WeakPtr<RHI::UniformBuffer> m_renderGraphConstantsBuffer;

		uint32_t m_currentPassIndex = 0;
		Weak<RenderGraphPassNodeBase> m_currentPassNode;
		RenderGraph* m_renderGraph = nullptr;

		RefPtr<RHI::CommandBuffer> m_commandBuffer;

		// #TODO_Ivar: Should be changed
		Vector<uint8_t> m_perPassConstantsBufferData;

#ifdef VT_DEBUG
		BoundPipelineData m_boundPipelineData;
#endif
	};

	template<typename T>
	inline void RenderContext::SetConstant(const StringHash& name, const T& data)
	{
		VT_PROFILE_FUNCTION();

		// #TODO_Ivar: Add validation
		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		const RHI::ShaderRenderGraphConstantsData& constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		VT_ENSURE(uniform.type == TryGetTypeFromType<T>());
		m_boundPipelineData.uniformHasBeenSetMap[name] = true;
#endif

		memcpy_s(&m_perPassConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, &data, sizeof(T));
	}

	template<typename F>
	inline void RenderContext::SetConstant(const StringHash& name, const Vector<F>& data)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);
		
		const RHI::ShaderRenderGraphConstantsData& constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		VT_ENSURE(uniform.type == TryGetTypeFromType<F>());
		m_boundPipelineData.uniformHasBeenSetMap[name] = true;
#endif

		memcpy_s(&m_perPassConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, data.data(), data.size() * sizeof(F));
	}

	template<typename F, size_t COUNT>
	inline void RenderContext::SetConstant(const StringHash& name, const std::array<F, COUNT>& data)
	{
		VT_PROFILE_FUNCTION();

		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		const RHI::ShaderRenderGraphConstantsData& constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		VT_ENSURE(uniform.type == TryGetTypeFromType<F>());
		m_boundPipelineData.uniformHasBeenSetMap[name] = true;
#endif

		memcpy_s(&m_perPassConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, data.data(), COUNT * sizeof(F));
	}
}
