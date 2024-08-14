#pragma once

#include "Volt/Asset/AssetTypes.h"

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetFactory.h>

namespace Volt
{
	class ShaderSource : public Asset
	{
	public:
		~ShaderSource() override = default;

		static AssetType GetStaticType() { return AssetTypes::ShaderSource; }
		VT_INLINE AssetType GetType() override { return GetStaticType(); }
		VT_INLINE uint32_t GetVersion() const override { return 1; }

		inline static constexpr std::string_view Extension = ".hlsl";
		inline static constexpr std::string_view ExtensionInclude = ".hlsli";

	private:
	};

	VT_REGISTER_ASSET_FACTORY(AssetTypes::ShaderSource, ShaderSource);
}
