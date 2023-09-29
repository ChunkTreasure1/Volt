#pragma once

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Reflection/VoltGUID.h>

#include <unordered_map>
#include <functional>
#include <typeindex>

namespace Volt
{
	class IComponentTypeDesc;
	class IEnumTypeDesc;
	class IArrayTypeDesc;
	struct ComponentMember;

	struct MonoScriptEntry;

	class MonoScriptInstance;
}

class ComponentPropertyUtility
{
public:
	static void DrawComponents(Weak<Volt::Scene> scene, Volt::Entity entity);
	static void DrawMonoScripts(Weak<Volt::Scene> scene, Volt::Entity entity);

private:
	static void Initialize();

	static void DrawComponent(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::IComponentTypeDesc* componentType, void* data, const size_t offset, bool isOpen, bool isSubSection);
	static void DrawComponentDefaultMember(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, void* data, const size_t offset);
	static void DrawComponentDefaultMemberArray(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& arrayMember, void* elementData, const size_t index, const std::type_index& typeIndex);
	static void DrawComponentEnum(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, const Volt::IEnumTypeDesc* enumType, void* data, const size_t offset);
	static void DrawComponentArray(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, const Volt::IArrayTypeDesc* arrayDesc, void* data, const size_t offset);

	static void DrawMonoScript(Weak<Volt::Scene> scene, const Volt::MonoScriptEntry& scriptEntry, Volt::Entity entity);
	static void DrawMonoMembers(Weak<Volt::Scene> scene, const Volt::MonoScriptEntry& scriptEntry, Volt::Entity entity);

	static void AddLocalChangeToEntity(Volt::Entity entity, const VoltGUID& componentGuid, std::string_view memberName);
	static void AddLocalChangeToEntity(Volt::Entity entity, const std::string& scriptName, std::string_view memberName);

	inline static bool s_initialized = false;
	inline static std::unordered_map<std::type_index, std::function<bool(std::string_view, void*, const size_t)>> s_propertyFunctions;
	inline static std::unordered_map<std::type_index, std::function<bool(const std::string&, Ref<Volt::MonoScriptInstance>)>> s_monoPropertyFunctions;

	ComponentPropertyUtility() = delete;
};
