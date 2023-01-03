#pragma once

#include "Volt/Rendering/RenderPass.h"
#include "Volt/Rendering/Shader/ShaderCommon.h"

#include <cstdint>

struct ID3D11DeviceContext;

namespace Volt
{
	enum class Context : uint32_t
	{
		Immidiate = 0,
		Deferred = 1
	};

	class SamplerState;
	class ConstantBuffer;
	class StructuredBuffer;
	class VertexBuffer;
	class IndexBuffer;

	class Image2D;

	class RenderCommand
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void SetTopology(Topology topology);
		static void SetCullState(CullState cullState);
		static void SetDepthState(DepthState depthState);
		static void SetContext(Context context);

		static void Draw(uint32_t vertexCount, uint32_t startVertexLocation);
		static void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation);
		static void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, uint32_t startInstanceLocation);
		static void Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ);

		static void BeginAnnotation(std::string_view name);
		static void EndAnnotation();

		static void Sampler_Bind(const SamplerState* samplerState, uint32_t slot);
		static void Sampler_BindMultiple(const std::vector<SamplerState*>& samplerStates, uint32_t startSlot);
		
		static void* ConstantBuffer_Map(const ConstantBuffer* constantBuffer);
		static void ConstantBuffer_Unmap(const ConstantBuffer* constantBuffer);

		static void* StructuredBuffer_Map(const StructuredBuffer* structuredBuffer);
		static void StructuredBuffer_Unmap(const StructuredBuffer* structuredBuffer);

		static void VertexBuffer_Bind(const VertexBuffer* vertexBuffer, uint32_t slot = 0, uint32_t stride = 0, uint32_t offset = 0);
		static void* VertexBuffer_Map(const VertexBuffer* vertexBuffer);
		static void VertexBuffer_Unmap(const VertexBuffer* vertexBuffer);

		static void IndexBuffer_Bind(const IndexBuffer* indexBuffer);

		static void BindTexturesToStage(const ShaderStage stage, const std::vector<Ref<Image2D>>& textures, const uint32_t startSlot = 0);
		static void BindComputeResources(const std::vector<Ref<Image2D>>& textures, const uint32_t startSlot = 0);
		static void ClearTexturesAtStage(const ShaderStage stage, const uint32_t startSlot, const uint32_t count);
		static void ClearComputeResources(const uint32_t startSlot, const uint32_t count);

		static ID3D11DeviceContext* GetCurrentContext();
		static void RestoreDefaultState();
	private:
		RenderCommand() = delete;
	};
}