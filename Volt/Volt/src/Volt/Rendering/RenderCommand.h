#pragma once

#include "Volt/Rendering/RenderPass.h"

#include <cstdint>

namespace Volt
{
	enum class Context : uint32_t
	{
		Immidiate = 0,
		Deferred = 1
	};

	class SamplerState;
	class ConstantBuffer;

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

		static void BeginAnnotation(std::string_view name);
		static void EndAnnotation();

		static void Sampler_Bind(SamplerState* samplerState, uint32_t slot);
		static void Sampler_BindMultiple(const std::vector<SamplerState*>& samplerStates, uint32_t startSlot);
		
		static void* ConstantBuffer_Map(ConstantBuffer* constantBuffer);
		static void ConstantBuffer_Unmap(ConstantBuffer* constantBuffer);

		static void RestoreDefaultState();
	private:
		RenderCommand() = delete;
	};
}