#pragma once

#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scene/Entity.h"

#include <string>
#include <unordered_map>
#include <typeindex>

#define VT_GET_MONO_ARRAY_TYPE(type) myRegisteredArrayTypes.at(typeid(type));
#define VT_REGISTER_MONO_ARRAY_TYPE(type, fullclassname) myRegisteredArrayTypes[typeid(type)] = fullclassname;

extern "C"
{
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}

namespace Volt
{
	class MonoScriptUtils
	{
	public:
		static std::string GetClassName2(const std::string& fullClassName);
		static std::string GetNameSpace(const std::string& fullClassName);

		static const std::string GetStringFromMonoString(MonoString* string);
		static MonoString* GetMonoStringFromString(const std::string& string);

		static FieldAccessibility GetFieldAccessabilityLevel(MonoClassField* field);

		template<typename T>
		static MonoArray* CreateMonoArray(const Vector<T>& vector);
		static MonoArray* CreateMonoArrayUInt32(const Vector<uint32_t>& vector);
		static MonoArray* CreateMonoArrayUInt64(const Vector<uint64_t>& vector);
		static MonoArray* CreateMonoArrayEntity(const Vector<EntityID>& vector);
		static MonoArray* CreateMonoArrayEntity(const Vector<Entity>& vector);

		static bool CreateNewCSFile(std::string name, std::filesystem::path directoryFromAssets, bool regenerate);

	private:
		friend class MonoScriptEngine;

		static MonoArray* InternalCreateMonoArray(const std::string& aNamespace, const std::string& aClass, const Vector<void*> data);
		static void RegisterArrayTypes();

		inline static std::unordered_map<std::type_index, std::string> myRegisteredArrayTypes;
	};


	// THIS IS NOT WORKING ATM
	// -----

	template<typename T>
	MonoArray* MonoScriptUtils::CreateMonoArray(const Vector<T>& vector)
	{
		// Get the namespace and class name for the specific type T.
		if (!MonoScriptUtils::myRegisteredArrayTypes.contains(typeid(T))) { return nullptr; }
		auto typeinfo = MonoScriptUtils::myRegisteredArrayTypes.at(typeid(T));

		// Fill with the data
		Vector<void*> data;

		return InternalCreateMonoArray(MonoScriptUtils::GetNameSpace(typeinfo), MonoScriptUtils::GetClassName2(typeinfo), data);
	}

	// -----
}
