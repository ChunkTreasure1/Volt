#pragma once

#include <Volt/Scene/Entity.h>

#include <CoreUtilities/VoltGUID.h>
#include <CoreUtilities/TypeTraits/TypeIndex.h>

#include <unordered_map>
#include <functional>

namespace Volt
{
	class IComponentTypeDesc;
	class IEnumTypeDesc;
	class IArrayTypeDesc;
	struct ComponentMember;
}

class ComponentPropertyUtility
{
public:
	static void DrawComponents(Weak<Volt::Scene> scene, Volt::Entity entity);

private:
	static void Initialize();

	static bool DrawComponent(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::IComponentTypeDesc* componentType, void* data, const size_t offset, bool isOpen, bool isSubSection);
	static bool DrawComponentDefaultMember(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, void* data, const size_t offset);
	static bool DrawComponentDefaultMemberArray(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& arrayMember, void* elementData, const size_t index, const TypeTraits::TypeIndex& typeIndex, AssetType arrayAssetType);
	static bool DrawComponentEnum(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, const Volt::IEnumTypeDesc* enumType, void* data, const size_t offset);
	static bool DrawComponentArray(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, const Volt::IArrayTypeDesc* arrayDesc, void* data, const size_t offset);

	static void AddLocalChangeToEntity(Volt::Entity entity, const VoltGUID& componentGuid, std::string_view memberName);

	inline static bool s_initialized = false;
	inline static std::unordered_map<TypeTraits::TypeIndex, std::function<bool(std::string_view, void*, const size_t)>> s_propertyFunctions;

	ComponentPropertyUtility() = delete;
};
