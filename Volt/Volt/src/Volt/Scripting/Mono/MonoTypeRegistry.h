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
		Color = BIT(0)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(MonoTypeFlags);

	struct MonoTypeInfo
	{
		std::type_index typeIndex = typeid(void);
		size_t typeSize = 0;

		AssetType assetType;
		MonoTypeFlags typeFlags;

		[[nodiscard]] inline const bool IsEntity() const { return typeIndex == typeid(entt::entity); }
		[[nodiscard]] inline const bool IsAsset() const { return typeIndex == typeid(AssetHandle) && assetType != AssetType::None; }
		[[nodiscard]] inline const bool IsString() const { return typeIndex == typeid(std::string); }
	};

	class MonoTypeRegistry
	{
	public:
		static void Initialize();

		static const MonoTypeInfo GetTypeInfo(std::string_view monoTypeName);
		static const MonoTypeInfo GetTypeInfo(const std::type_index& typeIndex);

	private:
		MonoTypeRegistry() = delete;

		inline static std::unordered_map<std::string_view, MonoTypeInfo> s_monoToNativeTypeMap;
	};
}
