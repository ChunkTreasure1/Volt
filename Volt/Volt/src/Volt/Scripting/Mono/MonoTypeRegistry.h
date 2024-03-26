#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Scene/EntityID.h"

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

	struct CustomMonoType
	{
		inline friend bool operator==(const CustomMonoType& lhs, const CustomMonoType& rhs)
		{
			return &rhs == &rhs;
		}
	};

	struct MonoTypeInfo
	{
		std::string typeName;
		std::type_index typeIndex = typeid(void);
		size_t typeSize = 0;

		AssetType assetType = AssetType::None;
		MonoTypeFlags typeFlags = MonoTypeFlags::None;
		
		std::function<bool(const void* lhs, const void* rhs)> equalFunc;

		[[nodiscard]] inline const bool IsEntity() const { return typeIndex == typeid(Volt::EntityID); }
		[[nodiscard]] inline const bool IsAsset() const { return typeIndex == typeid(AssetHandle) && assetType != AssetType::None; }
		[[nodiscard]] inline const bool IsString() const { return typeIndex == typeid(std::string); }
		[[nodiscard]] inline const bool IsEnum() const { return (typeFlags & MonoTypeFlags::Enum) != MonoTypeFlags::None; }
		[[nodiscard]] inline const bool IsCustomMonoType() const { return typeIndex == typeid(CustomMonoType); }
		[[nodiscard]] inline const bool IsValid() const { return typeSize != 0; }
	};

	class MonoTypeRegistry
	{
	public:
		static void Initialize();

		static const MonoTypeInfo& GetTypeInfo(std::string_view monoTypeName);
		static const MonoTypeInfo& GetTypeInfo(const std::type_index& typeIndex);

		static void RegisterEnum(const std::string& typeName);
		static void RegisterCustomType(const std::string& typeName);

	private:
		MonoTypeRegistry() = delete;

		inline static std::unordered_map<std::string, MonoTypeInfo> s_monoToNativeTypeMap;
	};
}
