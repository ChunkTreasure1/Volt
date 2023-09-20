#pragma once

#include "Volt/Asset/Asset.h"

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
		AssetType assetType;
		MonoTypeFlags typeFlags;
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
