#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Asset/Asset.h"

#include <functional>

namespace Volt
{
	class Asset;

	class AssetFactory
	{
	public:
		using AssetCreateFunction = std::function<Ref<Asset>()>;

		void Initialize();
		void Shutdown();

		Ref<Asset> CreateAssetOfType(AssetType type) const;

	private:
		std::unordered_map<AssetType, AssetCreateFunction> m_assetFactoryFunctions;
	};
}
