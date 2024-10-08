#include "aspch.h"
#include "AssetFactory.h"

Volt::AssetFactory g_assetFactory;

namespace Volt
{
	bool AssetFactory::RegisterAssetType(VoltGUID typeGuid, const AssetCreateFunction& func)
	{
		m_assetFactoryFunctions[typeGuid] = func;
		return true;
	}

	Ref<Asset> AssetFactory::CreateAssetOfType(AssetType type) const
	{
		VT_ENSURE(m_assetFactoryFunctions.contains(type->GetGUID()));
		return m_assetFactoryFunctions.at(type->GetGUID())();
	}
}
