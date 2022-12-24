#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Rendering/Vertex.h"

#include "Volt/Rendering/RenderCommand.h"

#include <vector>
#include <wrl.h>

struct ID3D11Buffer;

using namespace Microsoft::WRL;
namespace Volt
{
	enum class BufferUsage : uint32_t
	{
		Immutable = 1,
		Dynamic = 2
	};

	class VertexBuffer
	{
	public:
		VertexBuffer(const void* data, uint32_t aSize, uint32_t aStride, BufferUsage usage);
		VertexBuffer(uint32_t aSize, uint32_t aStride);
		~VertexBuffer();

		inline ComPtr<ID3D11Buffer> GetHandle() const { return myBuffer; }

		void SetName(const std::string& name);
		void SetData(const void* aData, uint32_t aSize);
		void Bind(uint32_t aSlot = 0) const;

		template<typename T>
		T* Map();
		void Unmap();

		static Ref<VertexBuffer> Create(const void* data, uint32_t aSize, uint32_t aStride, BufferUsage usage = BufferUsage::Immutable);
		static Ref<VertexBuffer> Create(uint32_t aSize, uint32_t aStride);
	
	private:
		ComPtr<ID3D11Buffer> myBuffer = nullptr;
		uint32_t myStride = 0;
	};
	
	template<typename T>
	inline T* VertexBuffer::Map()
	{
		return reinterpret_cast<T*>(RenderCommand::VertexBuffer_Map(this));
	}
}