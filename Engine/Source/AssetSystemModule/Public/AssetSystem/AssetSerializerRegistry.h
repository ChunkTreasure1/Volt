#pragma once

#include "AssetSystem/Config.h"
#include "AssetSystem/AssetType.h"
#include "AssetSystem/Serialization/AssetSerializer.h"

#include <unordered_map>

class VTAS_API AssetSerializerRegistry
{
public:
	bool RegisterAssetSerializer(VoltGUID typeGuid, Ref<Volt::AssetSerializer> serializer);

	Volt::AssetSerializer& GetSerializer(AssetType type) const;
	VT_INLINE bool HasSerializer(AssetType type) const { return m_serializers.contains(type->GetGUID()); }

private:
	vt::map<VoltGUID, Ref<Volt::AssetSerializer>> m_serializers;
};

extern VTAS_API AssetSerializerRegistry g_assetSerializerRegistry;

VT_NODISCARD VT_INLINE AssetSerializerRegistry& GetAssetSerializerRegistry()
{
	return g_assetSerializerRegistry;
}

#define VT_REGISTER_ASSET_SERIALIZER(type, serializer) \
	inline static bool AssetSerializer_ ## serializer ## _Registered = GetAssetSerializerRegistry().RegisterAssetSerializer(type ## Type::guid, CreateRef<serializer>())
