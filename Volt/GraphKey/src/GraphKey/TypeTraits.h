#pragma once

#include <unordered_map>
#include <typeindex>
#include <any>

#define GK_REGISTER_TYPE(x, stringName, uiFunction) template<> struct GraphKey::TypeTraits<x> \
	{ static const char* name; static bool registered; }; const char* GraphKey::TypeTraits<x>::name = #stringName; \
	bool GraphKey::TypeTraits<x>::registered = GraphKey::TypeRegistry::Register<x>(std::type_index(typeid(x)), #stringName, uiFunction); 

//#define GK_REGISTER_TYPE(x) template<> struct GraphKey::TypeTraits<x> \
//	{ static const char* name; static bool registered; }; const char* GraphKey::TypeTraits<x>::name = #x; \
//	bool GraphKey::TypeTraits<x>::registered = GraphKey::TypeRegistry::Register(std::type_index(typeid(x)), #x); 

namespace GraphKey
{
	template<typename T>
	struct TypeTraits;

	class TypeRegistry
	{
	public:
		struct TypeInfo
		{
			std::any defaultValue;
			std::type_index typeIndex = typeid(void);
			std::function<void(std::string name, std::any&)> uiFunction;
		};

		template<typename T>
		inline static bool Register(const std::type_index& index, const std::string& name, std::function<void(std::string name, std::any&)> uiFunction)
		{
			if (!GetNameRegistry().contains(index))
			{
				GetNameRegistry()[index] = name;
				GetTypeRegistry()[name].typeIndex = index;
				GetTypeRegistry()[name].defaultValue = T{};
				GetTypeRegistry()[name].uiFunction = uiFunction;
				return true;
			}

			return false;
		}

		inline static const std::string& GetNameFromTypeIndex(const std::type_index& index)
		{
			if (GetNameRegistry().contains(index))
			{
				return GetNameRegistry().at(index);
			}

			static std::string empty;
			return empty;
		}

		inline static const std::type_index GetTypeIndexFromName(const std::string& name)
		{
			if (GetTypeRegistry().contains(name))
			{
				return GetTypeRegistry().at(name).typeIndex;
			}

			return std::type_index(typeid(void));
		}

		inline static const std::any GetDefaultValueFromName(const std::string& name)
		{
			if (GetTypeRegistry().contains(name))
			{
				return GetTypeRegistry().at(name).defaultValue;
			}

			return {};
		}

		inline static void ExecuteUIFunctionOfType(const std::string& name, std::any& value)
		{
			const auto typeIndex = std::type_index(value.type());

			if (!GetNameRegistry().contains(typeIndex))
			{
				return;
			}

			const auto typeName = GetNameRegistry().at(typeIndex);

			if (!GetTypeRegistry().contains(typeName))
			{
				return;
			}

			GetTypeRegistry().at(typeName).uiFunction(name, value);
		}

		inline static std::unordered_map<std::string, TypeInfo>& GetTypeRegistry()
		{
			static std::unordered_map<std::string, TypeInfo> registry;
			return registry;
		}

	private:
		inline static std::unordered_map<std::type_index, std::string>& GetNameRegistry()
		{
			static std::unordered_map<std::type_index, std::string> registry;
			return registry;
		}
	};
}