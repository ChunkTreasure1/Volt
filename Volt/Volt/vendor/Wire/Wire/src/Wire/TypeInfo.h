#pragma once

#include <typeindex>
#include <functional>
#include <string>

#define W_REGISTER_TYPE(x, stringName, uiFunction) template<> struct Wire::TypeTraits<x> \
	{  \
		static const char* name; \
		static const char* prettyName; \
		static bool registered; \
		static size_t size; \
	}; \
	const char* Wire::TypeTraits<x>::prettyName = #stringName; \
	const char* Wire::TypeTraits<x>::name = #x; \
	size_t Wire::TypeTraits<x>::size = sizeof(x) \
	bool Wire::TypeTraits<x>::registered = Wire::TypeRegistry::Register<x>(#x, #stringName, uiFunction); 

namespace Wire
{
	template<typename T>
	struct TypeTraits;

	class TypeRegistry
	{
	public:
		struct TypeInfo
		{
			size_t size = 0;
			std::string prettyName;
			std::string typeName;
			std::type_index typeIndex = typeid(void);
			std::function<void(const std::string& name, uint8_t* data)> uiFunction;
		};

		template<typename T>
		inline static bool Register(const std::string& typeName, const std::string& prettyName, std::function<void(const std::string& name)> uiFunction)
		{
			const auto index = typeid(T);

			if (!GetNameRegistry().contains(index))
			{
				GetNameRegistry()[index] = prettyName;
				GetTypeRegistry()[prettyName].typeIndex = index;
				GetTypeRegistry()[prettyName].uiFunction = uiFunction;
				GetTypeRegistry()[prettyName].prettyName = prettyName;
				GetTypeRegistry()[prettyName].typeName = typeName;
				GetTypeRegistry()[prettyName].size = sizeof(T);
				return true;
			}

			return false;
		}

		inline static std::unordered_map<std::string, TypeInfo>& GetTypeRegistry()
		{
			static std::unordered_map<std::string, TypeInfo> registry;
			return registry;
		}

		static const TypeInfo& GetTypeInfoFromPrettyName(const std::string& name);
		static const TypeInfo& GetTypeInfoFromTypeName(const std::string& name);
		static const TypeInfo& GetTypeInfoFromTypeIndex(const std::type_index& typeIndex);

	private:
		inline static std::unordered_map<std::type_index, std::string>& GetNameRegistry()
		{
			static std::unordered_map<std::type_index, std::string> registry;
			return registry;
		}
	};
}