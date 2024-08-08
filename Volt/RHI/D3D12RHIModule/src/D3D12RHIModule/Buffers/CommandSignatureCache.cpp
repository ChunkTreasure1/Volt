#include "dxpch.h"
#include "CommandSignatureCache.h"

#include <RHIModule/Core/RHICommon.h>
#include <RHIModule/Graphics/GraphicsDevice.h>
#include <RHIModule/Graphics/GraphicsContext.h>

#include <RHIModule/Utility/HashUtility.h>

#include <d3d12/d3d12.h>

namespace Volt::RHI
{
	VT_INLINE ComPtr<ID3D12CommandSignature> CreateCommandSignature(D3D12_INDIRECT_ARGUMENT_TYPE argType, uint32_t stride)
	{
		auto d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();

		D3D12_INDIRECT_ARGUMENT_DESC argDesc{};
		argDesc.Type = argType;

		D3D12_COMMAND_SIGNATURE_DESC signatureDesc{};
		signatureDesc.pArgumentDescs = &argDesc;
		signatureDesc.NumArgumentDescs = 1;
		signatureDesc.ByteStride = stride;

		ComPtr<ID3D12CommandSignature> result;
		VT_D3D12_CHECK(d3d12Device->CreateCommandSignature(&signatureDesc, nullptr, VT_D3D12_WRID(result)));

		return result;
	}

	template<D3D12_INDIRECT_ARGUMENT_TYPE ARG_TYPE, typename TYPE>
	VT_INLINE ComPtr<ID3D12CommandSignature> CreateCommandSignature()
	{
		return CreateCommandSignature(ARG_TYPE, sizeof(TYPE));
	}

	VT_INLINE size_t GetCommandSignatureHash(const CommandSignatureType type, const uint32_t stride)
	{
		size_t result = std::hash<uint8_t>()(static_cast<uint8_t>(type));
		result = Math::HashCombine(result, std::hash<uint32_t>()(stride));
		return result;
	}

	VT_INLINE D3D12_INDIRECT_ARGUMENT_TYPE GetTypeFromSignatureType(CommandSignatureType type)
	{
		switch (type)
		{
			case CommandSignatureType::Draw: return D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
			case CommandSignatureType::DrawIndexed: return D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
			case CommandSignatureType::Dispatch: return D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
			case CommandSignatureType::DispatchRays: return D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS;
			case CommandSignatureType::DispatchMesh: return D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;
		}

		VT_ENSURE(false);
		return D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
	}

	CommandSignatureCache::CommandSignatureCache()
	{
		VT_ENSURE(s_instance == nullptr);
		s_instance = this;

		m_signatureCache[GetCommandSignatureHash(CommandSignatureType::Draw, sizeof(IndirectDrawCommand))] = CreateCommandSignature<D3D12_INDIRECT_ARGUMENT_TYPE_DRAW, IndirectDrawCommand>();
		m_signatureCache[GetCommandSignatureHash(CommandSignatureType::DrawIndexed, sizeof(IndirectDrawIndexedCommand))] = CreateCommandSignature<D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, IndirectDrawIndexedCommand>();
		m_signatureCache[GetCommandSignatureHash(CommandSignatureType::Dispatch, sizeof(IndirectDispatchCommand))] = CreateCommandSignature<D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH, IndirectDispatchCommand>();
	}
	
	CommandSignatureCache::~CommandSignatureCache()
	{
		m_signatureCache.clear();
		s_instance = nullptr;
	}

	ComPtr<ID3D12CommandSignature> CommandSignatureCache::GetOrCreateCommandSignature(CommandSignatureType type, const uint32_t stride) const
	{
		const size_t hash = GetCommandSignatureHash(type, stride);

		std::scoped_lock lock{ m_cacheMutex };

		if (m_signatureCache.contains(hash))
		{
			return m_signatureCache.at(hash);
		}

		m_signatureCache[hash] = CreateCommandSignature(GetTypeFromSignatureType(type), stride);
		return m_signatureCache.at(hash);
	}
}
