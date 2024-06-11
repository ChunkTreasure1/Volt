#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class ShaderSource : public Asset
	{
	public:
		~ShaderSource() override = default;

		static AssetType GetStaticType() { return AssetType::ShaderSource; }
		VT_INLINE AssetType GetType() override { return GetStaticType(); }
		VT_INLINE uint32_t GetVersion() const override { return 1; }

	private:
	};
}
