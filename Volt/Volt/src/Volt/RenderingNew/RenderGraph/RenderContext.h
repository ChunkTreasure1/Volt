#pragma once

#include "Volt/Core/Base.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphCommon.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "Volt/RenderingNew/Resources/ResourceHandle.h"

#include <VoltRHI/Core/RHICommon.h>
#include <VoltRHI/Shader/Shader.h>
	
#include <glm/glm.hpp>
#include <half/half.hpp>

namespace Volt
{
	namespace RHI
	{
		class CommandBuffer;
		class StorageBuffer;
		class VertexBuffer;
		class IndexBuffer;

		class RenderPipeline;
		class ComputePipeline;
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

	class RenderContext
	{
	public:
		RenderContext(Ref<RHI::CommandBuffer> commandBuffer);
		~RenderContext();

		void BeginRendering(const RenderingInfo& renderingInfo);
		void EndRendering();

		const RenderingInfo CreateRenderingInfo(const uint32_t width, const uint32_t height, const std::vector<Ref<RHI::ImageView>>& attachments);

		void ClearImage(Ref<RHI::Image2D> image, const glm::vec4& clearColor);
		void ClearBuffer(Ref<RHI::StorageBuffer> buffer, uint32_t clearValue);

		void UploadBufferData(Ref<RHI::StorageBuffer> buffer, const void* data, const size_t size);

		void DispatchMeshTasks(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ);
		void DispatchMeshTasksIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride);
		void DispatchMeshTasksIndirectCount(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, Ref<RHI::StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride);

		void Dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ);
		void DispatchIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset);
		
		void DrawIndirectCount(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, Ref<RHI::StorageBuffer> countBuffer, const size_t countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride);
		void DrawIndexedIndirect(Ref<RHI::StorageBuffer> commandsBuffer, const size_t offset, const uint32_t drawCount, const uint32_t stride);
		void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance);
		void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance);

		void BindPipeline(Ref<RHI::RenderPipeline> pipeline);
		void BindPipeline(Ref<RHI::ComputePipeline> pipeline);

		void BindIndexBuffer(Ref<RHI::StorageBuffer> indexBuffer);
		void BindIndexBuffer(Ref<RHI::IndexBuffer> indexBuffer);
		void BindVertexBuffers(const std::vector<Ref<RHI::VertexBuffer>>& vertexBuffers, const uint32_t firstBinding);

		void Flush();
		Ref<RHI::StorageBuffer> GetReadbackBuffer(Ref<RHI::StorageBuffer> buffer);

		template<typename T>
		void SetConstant(const std::string& name, const T& data);

		template<>
		void SetConstant(const std::string& name, const ResourceHandle& data);

		template<typename F>
		void SetConstant(const std::string& name, const std::vector<F>& data);

		template<typename F, size_t COUNT>
		void SetConstant(const std::string& name, const std::array<F, COUNT>& data);

		void SetConstant(const void* data, uint32_t size);

	private:
		friend class RenderGraph;

		void BindDescriptorTableIfRequired();

		void SetPassConstantsBuffer(Weak<RHI::StorageBuffer> constantsBuffer);
		void SetCurrentPassIndex(Weak<RenderGraphPassNodeBase> currentPassNode);
		void UploadConstantsData();

		RHI::ShaderRenderGraphConstantsData GetRenderGraphConstantsData();

		Ref<RHI::DescriptorTable> GetOrCreateDescriptorTable(Ref<RHI::RenderPipeline> renderPipeline);
		Ref<RHI::DescriptorTable> GetOrCreateDescriptorTable(Ref<RHI::ComputePipeline> computePipeline);

		// Internal state
		bool m_descriptorTableIsBound = false; // This needs to be checked in every call that uses resources

		Weak<RHI::RenderPipeline> m_currentRenderPipeline;
		Weak<RHI::ComputePipeline> m_currentComputePipeline;
		Ref<RHI::DescriptorTable> m_currentDescriptorTable;

		Weak<RHI::StorageBuffer> m_passConstantsBuffer;

		uint32_t m_currentPassIndex = 0;
		Weak<RenderGraphPassNodeBase> m_currentPassNode;

		Ref<RHI::CommandBuffer> m_commandBuffer;

		std::unordered_map<void*, Ref<RHI::DescriptorTable>> m_descriptorTableCache;
	
		// #TODO_Ivar: Should be changed
		std::vector<uint8_t> m_passConstantsBufferData;
	};

	template<typename T>
	inline void RenderContext::SetConstant(const std::string& name, const T& data)
	{
		// #TODO_Ivar: Add validation
		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		RHI::ShaderRenderGraphConstantsData constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		VT_ENSURE(uniform.type == TryGetTypeFromType<T>());
#endif

		memcpy_s(&m_passConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, &data, sizeof(T));
	}

	template<typename F>
	inline void RenderContext::SetConstant(const std::string& name, const std::vector<F>& data)
	{
		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);
		
		RHI::ShaderRenderGraphConstantsData constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		VT_ENSURE(uniform.type == TryGetTypeFromType<F>());
#endif

		memcpy_s(&m_passConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, data.data(), data.size() * sizeof(F));
	}

	template<typename F, size_t COUNT>
	inline void RenderContext::SetConstant(const std::string& name, const std::array<F, COUNT>& data)
	{
		VT_ENSURE(m_currentRenderPipeline || m_currentComputePipeline);

		RHI::ShaderRenderGraphConstantsData constantsData = GetRenderGraphConstantsData();
		VT_ENSURE(constantsData.uniforms.contains(name));

		const auto& uniform = constantsData.uniforms.at(name);

#ifdef VT_DEBUG
		VT_ENSURE(uniform.type == TryGetTypeFromType<F>());
#endif

		memcpy_s(&m_passConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + uniform.offset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE, data.data(), COUNT * sizeof(F));
	}

	inline void RenderContext::SetConstant(const void* data, uint32_t size)
	{
		// #TODO_Ivar: Add validation

		memcpy_s(&m_passConstantsBufferData[m_currentPassIndex * RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE + m_currentPassConstantsOffset], RenderGraphCommon::MAX_PASS_CONSTANTS_SIZE - m_currentPassConstantsOffset, &data, size);
		m_currentPassConstantsOffset += size;
	}
}
