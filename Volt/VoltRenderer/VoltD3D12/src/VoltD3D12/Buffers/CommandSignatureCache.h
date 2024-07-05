#pragma once

#include "VoltD3D12/Common/ComPtr.h"

#include <CoreUtilities/Core.h>
#include <CoreUtilities/Containers/Map.h>

struct ID3D12CommandSignature;

namespace Volt::RHI
{
	enum class CommandSignatureType : uint8_t
	{
		Draw,
		DrawIndexed,
		Dispatch,
		DispatchRays,
		DispatchMesh
	};

	class CommandSignatureCache
	{
	public:
		CommandSignatureCache();
		~CommandSignatureCache();

		VT_NODISCARD VT_INLINE static CommandSignatureCache& Get() { return *s_instance; }

		ComPtr<ID3D12CommandSignature> GetOrCreateCommandSignature(CommandSignatureType type, const uint32_t stride) const;

	private:
		inline static CommandSignatureCache* s_instance;

		mutable std::mutex m_cacheMutex;
		mutable vt::map<size_t, ComPtr<ID3D12CommandSignature>> m_signatureCache;
	};
}
