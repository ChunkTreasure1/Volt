#pragma once

#include "AssetSystem/Asset.h"

#include <functional>

namespace Volt
{
	class Asset;

	class VTAS_API AssetFactory
	{
	public:
		using AssetCreateFunction = std::function<Ref<Asset>()>;

		bool RegisterAssetType(VoltGUID typeGuid, const AssetCreateFunction& func);
		VT_NODISCARD Ref<Asset> CreateAssetOfType(AssetType type) const;

	private:
		std::unordered_map<VoltGUID, AssetCreateFunction> m_assetFactoryFunctions;
	};

}

extern VTAS_API Volt::AssetFactory g_assetFactory;

VT_NODISCARD VT_INLINE Volt::AssetFactory& GetAssetFactory()
{
	return g_assetFactory;
}

#define VT_REGISTER_ASSET_FACTORY(assetType, type) \
	inline static bool AssetFactory_ ## type ## _Registered = GetAssetFactory().RegisterAssetType(assetType ## Type ##::guid, []() { return CreateRef<type>(); })
