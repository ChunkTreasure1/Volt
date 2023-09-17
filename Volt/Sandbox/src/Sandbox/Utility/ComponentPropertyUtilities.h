#pragma once

#include <Volt/Scene/Entity.h>

#include <unordered_map>
#include <functional>
#include <typeindex>

namespace Volt
{
	class IComponentTypeDesc;
	class IEnumTypeDesc;
	struct ComponentMember;
}

class ComponentPropertyUtility
{
public:
	static void DrawComponentProperties(Weak<Volt::Scene> scene, Volt::Entity entity);

private:
	static void Initialize();

	static void DrawComponent(Weak<Volt::Scene> scene, const Volt::IComponentTypeDesc* componentType, void* data);
	static void DrawComponentSubSection(Weak<Volt::Scene> scene, const Volt::IComponentTypeDesc* componentType, void* data, const size_t offset);
	static void DrawComponentDefaultMember(Weak<Volt::Scene> scene, const Volt::ComponentMember& member, void* data, const size_t offset);
	static void DrawComponentEnum(Weak<Volt::Scene> scene, const Volt::ComponentMember& member, const Volt::IEnumTypeDesc* enumType, void* data, const size_t offset);

	inline static bool s_initialized = false;
	inline static std::unordered_map<std::type_index, std::function<bool(std::string_view, void*, const size_t)>> s_propertyFunctions;

	ComponentPropertyUtility() = delete;
};
