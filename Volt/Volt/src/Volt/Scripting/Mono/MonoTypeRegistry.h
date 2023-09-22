#pragma once

#include "Volt/Asset/Asset.h"

#include <entt.hpp>

#include <unordered_map>
#include <string_view>
#include <typeindex>

namespace Volt
{
	enum class MonoTypeFlags
	{
		None = 0,
		Color = BIT(0),
		Enum = BIT(1)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(MonoTypeFlags);

	struct MonoTypeInfo
	{
		std::string typeName;
		std::type_index typeIndex = typeid(void);
		size_t typeSize = 0;

		AssetType assetType;
		MonoTypeFlags typeFlags;

		[[nodiscard]] inline const bool IsEntity() const { return typeIndex == typeid(entt::entity); }
		[[nodiscard]] inline const bool IsAsset() const { return typeIndex == typeid(AssetHandle) && assetType != AssetType::None; }
		[[nodiscard]] inline const bool IsString() const { return typeIndex == typeid(std::string); }
		[[nodiscard]] inline const bool IsEnum() const { return (typeFlags & MonoTypeFlags::Enum) != MonoTypeFlags::None; }
	};

	class MonoTypeRegistry
	{
	public:
		static void Initialize();

		static const MonoTypeInfo GetTypeInfo(std::string_view monoTypeName);
		static const MonoTypeInfo GetTypeInfo(const std::type_index& typeIndex);

		static void RegisterEnum(const std::string& typeName);

	private:
		MonoTypeRegistry() = delete;

		inline static std::unordered_map<std::string, MonoTypeInfo> s_monoToNativeTypeMap;
	};
}
