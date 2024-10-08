
#pragma once

#include <EntitySystem/EntityID.h>
#include <EntitySystem/ComponentRegistry.h>
#include <EntitySystem/Scripting/ECSAccessBuilder.h>

#include <EntitySystem/Scripting/CommonComponent.h>
#include <EntitySystem/Scripting/CoreComponents.h>

#include <Volt/Asset/AssetTypes.h>

#include <glm/glm.hpp>

#include <string>
#include <string_view>

namespace Volt
{
	struct PrefabComponentLocalChange
	{
		VoltGUID componentGUID = VoltGUID::Null();
		std::string memberName;
		
		static void ReflectType(TypeDesc<PrefabComponentLocalChange>& reflect)
		{
			reflect.SetGUID("{C78B94DD-B814-4155-B9DE-072B91DE02B3}"_guid);
			reflect.SetLabel("Prefab Component Local Change");
			reflect.AddMember(&PrefabComponentLocalChange::componentGUID, "componentGUID", "Component GUID", "", VoltGUID::Null());
			reflect.AddMember(&PrefabComponentLocalChange::memberName, "memberName", "Member Name", "", std::string(""));
		}
	};

	struct PrefabComponent
	{
		AssetHandle prefabAsset = Asset::Null();
		EntityID prefabEntity = EntityID(0);
		EntityID sceneRootEntity = EntityID(0);
		uint32_t version = 0;

		Vector<PrefabComponentLocalChange> componentLocalChanges;

		bool isDirty = false;

		static void ReflectType(TypeDesc<PrefabComponent>& reflect)
		{
			reflect.SetGUID("{B8A83ACF-F1CA-4C9F-8D1E-408B5BB388D2}"_guid);
			reflect.SetLabel("Prefab Component");
			reflect.SetHidden();
			reflect.AddMember(&PrefabComponent::prefabAsset, "prefabAsset", "Prefab Asset", "", Asset::Null(), AssetTypes::Prefab);
			reflect.AddMember(&PrefabComponent::prefabEntity, "prefabEntity", "Prefab Entity", "", EntityID(0));
			reflect.AddMember(&PrefabComponent::sceneRootEntity, "sceneRootEntity", "Scene Root Entity", "", EntityID(0));
			reflect.AddMember(&PrefabComponent::version, "version", "Version", "", 0);
			reflect.AddMember(&PrefabComponent::componentLocalChanges, "componentLocalChanges", "Component Local Changes", "", Vector<PrefabComponentLocalChange>{});
		}

		REGISTER_COMPONENT(PrefabComponent);
	};
}
