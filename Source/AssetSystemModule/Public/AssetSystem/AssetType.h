#pragma once

#include "AssetSystem/Config.h"

#include <CoreUtilities/VoltGUID.h>
#include <CoreUtilities/Containers/Map.h>

#include <string>

class VTAS_API AssetTypeBase
{
public:
	inline static constexpr VoltGUID guid = VoltGUID::Null();
	inline static constexpr std::string_view name = "None";
	inline static constexpr bool isSourceType = false;
	inline static Vector<std::string> extensions = {};

	virtual ~AssetTypeBase() = default;
	VT_INLINE constexpr virtual std::string_view GetName() const { return name; }
	VT_INLINE constexpr virtual const VoltGUID& GetGUID() const { return guid; };
	VT_INLINE constexpr virtual bool IsSourceType() const { return isSourceType; }
	VT_INLINE constexpr virtual const Vector<std::string>& GetExtensions() const { return extensions; }

	VT_INLINE bool operator==(const AssetTypeBase& rhs) const
	{
		return GetGUID() == rhs.GetGUID();
	}

	VT_INLINE bool operator<(const AssetTypeBase& rhs) const
	{
		return GetGUID() < rhs.GetGUID();
	}
};

typedef Ref<AssetTypeBase> AssetType;

class VTAS_API AssetTypeRegistry
{
public:
	bool RegisterAssetType(const VoltGUID& guid, AssetType type);

	AssetType GetTypeFromGUID(const VoltGUID& guid) const;
	AssetType GetTypeFromExtension(const std::string& extension) const;
	VT_INLINE const vt::map<VoltGUID, AssetType>& GetTypeMap() const { return m_typeMap; }

private:
	vt::map<VoltGUID, AssetType> m_typeMap;
};

extern VTAS_API AssetTypeRegistry g_assetTypeRegistry;

VT_INLINE AssetTypeRegistry& GetAssetTypeRegistry()
{
	return g_assetTypeRegistry;
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<AssetTypeBase>
	{
		std::size_t operator()(const AssetTypeBase& handle) const
		{
			return std::hash<VoltGUID>()(handle.GetGUID());
		}
	};
}

#define VT_DECLARE_ASSET_TYPE_IMPL(typeName, isSourceTypeVal, typeExtensions, typeGuid) \
namespace AssetTypes \
{ \
	class typeName ## Type : public ::AssetTypeBase \
	{ \
	public: \
		~typeName ## Type() override = default; \
		inline static constexpr VoltGUID guid = typeGuid; \
		inline static constexpr std::string_view name = #typeName; \
		inline static constexpr bool isSourceType = isSourceTypeVal; \
		inline static Vector<std::string> extensions = typeExtensions; \
		VT_INLINE constexpr std::string_view GetName() const override { return name; } \
		VT_INLINE constexpr const VoltGUID& GetGUID() const override { return guid; } \
		VT_INLINE constexpr bool IsSourceType() const override { return isSourceType; } \
		VT_INLINE constexpr const Vector<std::string>& GetExtensions() const override { return extensions; } \
	}; \
	extern Ref<typeName ## Type> typeName; \
} \
extern bool AssetType_ ## typeName ## _Registered;

#define VT_DECLARE_ASSET_TYPE_EXPORT_IMPL(typeName, isSourceTypeVal, typeExtensions, typeGuid, exportKeyword) \
namespace AssetTypes \
{ \
	class exportKeyword typeName ## Type : public ::AssetTypeBase \
	{ \
	public: \
		~typeName ## Type() override = default; \
		inline static constexpr VoltGUID guid = typeGuid; \
		inline static constexpr std::string_view name = #typeName; \
		inline static constexpr bool isSourceType = isSourceTypeVal; \
		inline static Vector<std::string> extensions = typeExtensions; \
		VT_INLINE constexpr std::string_view GetName() const override { return name; } \
		VT_INLINE constexpr const VoltGUID& GetGUID() const override { return guid; } \
		VT_INLINE constexpr bool IsSourceType() const override { return isSourceType; } \
		VT_INLINE constexpr const Vector<std::string>& GetExtensions() const override { return extensions; } \
	}; \
	extern exportKeyword Ref<typeName ## Type> typeName; \
}

#define VT_DECLARE_ASSET_TYPE(typeName, typeGuid) \
	VT_DECLARE_ASSET_TYPE_IMPL(typeName, false, (Vector<std::string>{}), typeGuid)

#define VT_DECLARE_ASSET_SOURCE_TYPE(typeName, extensions, typeGuid) \
	VT_DECLARE_ASSET_TYPE_IMPL(typeName, true, extensions, typeGuid)

#define EXPAND(...) __VA_OPT__(__VA_ARGS__)

#define VT_REGISTER_ASSET_TYPE(typeName) \
	namespace AssetTypes { Ref<typeName ## Type> typeName = CreateRef<typeName ## Type>(); } \
	bool AssetType_ ## typeName ## _Registered = GetAssetTypeRegistry().RegisterAssetType(AssetTypes::typeName ## Type::guid, AssetTypes::typeName);

VT_DECLARE_ASSET_TYPE_EXPORT_IMPL(None, false, (Vector<std::string>{}), VoltGUID::Null(), VTAS_API);
