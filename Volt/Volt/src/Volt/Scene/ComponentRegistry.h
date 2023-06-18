#pragma once

#include "Volt/Asset/Asset.h"

#include <entt.hpp>
#include <typeindex>

#define UNPACK(...) __VA_ARGS__
#define ENTT_COMPONENT(compType) inline static bool _comp_registered = Volt::ComponentRegistry::RegisterComponent<compType>(#compType)
#define ENTT_PROPERTY(compType, var, prettyName, ...) inline static const entt::meta_factory<compType> _ ## compType ## var ## _factory = entt::meta<compType>().data<&compType::var>(prettyName ## _hs); \
													  inline static bool _ ## var ## _registered = Volt::ComponentRegistry::RegisterPropertyToComponent(#compType, prettyName, #compType ## "," ## #var ## "," ## prettyName ## "," #__VA_ARGS__) \

namespace Volt
{
	constexpr std::string_view SPECIAL_TYPE_NAME = "specialtype";
	enum SpecialType
	{
		eST_None,
		eST_Color,
		eST_Asset,
		eST_Directory
	};

	constexpr std::string_view VISIBLE_NAME = "visible";
	enum Visible
	{
		eV_True,
		eV_False
	};

	constexpr std::string_view SERIALIZABLE_NAME = "serializable";
	enum Serializable
	{
		eS_True,
		eS_False
	};

	constexpr std::string_view ASSET_TYPE_NAME = "assettype";

	struct ComponentProperty
	{
		std::string name;
		SpecialType specialType = eST_None;
		Visible visible = eV_True;
		Serializable serializable = eS_True;

		AssetType assetType = AssetType::None;
	};

	struct ComponentInfo
	{
		std::type_index typeIndex = typeid(void);
		std::vector<ComponentProperty> properties;
	};

	class ComponentRegistry
	{
	public:
		template<typename T>
		static bool RegisterComponent(std::string_view componentName);
		
		static bool RegisterPropertyToComponent(std::string_view componentName, std::string_view propertyName, std::string_view propertyString);

	private:
		static void ParsePropertyInfo(std::string_view propertyString, ComponentProperty& outProperty);
		static void ParseSpecialType(std::string_view specialTypeString, const std::vector<std::string>& propertyValues, ComponentProperty& outProperty);

		static std::string FindValueInString(std::string_view srcString, std::string_view key);

		inline static std::unordered_map<std::string_view, ComponentInfo> s_componentInfo{};
	};

	template<typename T>
	inline bool ComponentRegistry::RegisterComponent(std::string_view componentName)
	{
		s_componentInfo[componentName] = {};
		s_componentInfo[componentName].typeIndex = typeid(T);

		return true;
	}
}
