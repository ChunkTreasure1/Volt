#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/Particles/Particle.h"
#include "Volt/Animation/AnimationTreeController.h"

#include <Wire/Serialization.h>
#include <gem/gem.h>
#include <string>

namespace Volt
{
	class AnimationStateMachine;

	SERIALIZE_COMPONENT((struct TagComponent
	{
		PROPERTY(Name = Tag) std::string tag;

		CREATE_COMPONENT_GUID("{282FA5FB-6A77-47DB-8340-3D34F1A1FBBD}"_guid);
	}), TagComponent);

	SERIALIZE_COMPONENT((struct TransformComponent
	{
		PROPERTY(Name = Position) gem::vec3 position;
		PROPERTY(Name = Rotation) gem::vec3 rotation;
		PROPERTY(Name = Scale) gem::vec3 scale;

		PROPERTY(Visible = false) bool visible = true;
		PROPERTY(Visible = false) bool locked = false;

		inline const gem::mat4 GetTransform() const
		{
			return gem::translate(gem::mat4(1.f), position) *
				gem::mat4_cast(gem::quat(rotation)) * gem::scale(gem::mat4(1.f), scale);
		}

		inline const gem::vec3 GetForward() const
		{
			const gem::quat orientation = gem::quat(rotation);
			return gem::rotate(orientation, gem::vec3{ 0.f, 0.f, 1.f });
		}

		inline const gem::vec3 GetRight() const
		{
			const gem::quat orientation = gem::quat(rotation);
			return gem::rotate(orientation, gem::vec3{ 1.f, 0.f, 0.f });
		}

		inline const gem::vec3 GetUp() const
		{
			const gem::quat orientation = gem::quat(rotation);
			return gem::rotate(orientation, gem::vec3{ 0.f, 1.f, 0.f });
		}

		CREATE_COMPONENT_GUID("{E1B8016B-1CAA-4782-927E-C17C29B25893}"_guid);
	}), TransformComponent);

	SERIALIZE_COMPONENT((struct RelationshipComponent
	{
		PROPERTY(Name = Children) std::vector<Wire::EntityId> Children;
		PROPERTY(Name = Parent) Wire::EntityId Parent = 0;
		PROPERTY(Name = SortId, Visible = false) uint32_t sortId = 0;
		CREATE_COMPONENT_GUID("{4A5FEDD2-4D0B-4696-A9E6-DCDFFB25B32C}"_guid);

	}), RelationshipComponent);

	SERIALIZE_COMPONENT((struct PrefabComponent
	{
		PROPERTY(Name = PrefabAsset, Visible = false) AssetHandle prefabAsset = Asset::Null();
		PROPERTY(Name = prefabEntity, Visible = false) Wire::EntityId prefabEntity = Wire::NullID;
		PROPERTY(Name = Version, Visible = false) uint32_t version = 0;

		bool isDirty = false;

		CREATE_COMPONENT_GUID("{B8A83ACF-F1CA-4C9F-8D1E-408B5BB388D2}"_guid);
	}), PrefabComponent);

	SERIALIZE_COMPONENT((struct ParticleEmitterComponent
	{
		PROPERTY(Name = Preset, SpecialType = Asset, AssetType = ParticlePreset) AssetHandle preset;
		AssetHandle currentPreset;

		bool pressedPlay = false;
		bool isLooping = false;
		float emittionTimer = 0;
		int maxSpawnAmount = 0;
		int numberOfAliveParticles = 0;
		std::vector<Particle> particles;

		CREATE_COMPONENT_GUID("{E31271AB-47C7-4D6A-91E8-4B1A62B20D66}"_guid);
	}), ParticleEmitterComponent);

	SERIALIZE_COMPONENT((struct MeshComponent
	{
		PROPERTY(Name = Mesh, SpecialType = Asset, AssetType = Mesh) AssetHandle handle = Asset::Null();
		PROPERTY(Name = Material, SpecialType = Asset, AssetType = Material) AssetHandle overrideMaterial = Asset::Null();
		PROPERTY(Name = Walkable) bool walkable = false;
		PROPERTY(Name = Cast Shadows) bool castShadows = true;
		PROPERTY(Name = Cast AO) bool castAO = true;

		int32_t subMeshIndex = -1;
		int32_t subMaterialIndex = -1;

		CREATE_COMPONENT_GUID("{45D008BE-65C9-4D6F-A0C6-377F7B384E47}"_guid)
	}), MeshComponent);

	SERIALIZE_COMPONENT((struct DecalComponent
	{
		PROPERTY(Name = Material, SpecialType = Asset, AssetType = Material) AssetHandle materialHandle = Asset::Null();

		CREATE_COMPONENT_GUID("{09FA1C73-D508-4ADA-A101-A63703E91345}"_guid)
	}), DecalComponent);

	SERIALIZE_COMPONENT((struct ScriptedEventComponent
	{
		PROPERTY(Name = Spawn Radius) float spawnRadius = 500.f;
		PROPERTY(Name = Meele1) int w1MeeleAmount = 1;
		PROPERTY(Name = Ranged1) int w1RangedAmount = 1;
		PROPERTY(Name = Meele2) int w2MeeleAmount = 1;
		PROPERTY(Name = Ranged2) int w2RangedAmount = 1;
		PROPERTY(Name = Meele3) int w3MeeleAmount = 1;
		PROPERTY(Name = Ranged3) int w3RangedAmount = 1;

		CREATE_COMPONENT_GUID("{CD8FEFA9-4AE8-457D-B8D5-A2A2AB52F387}"_guid)
	}), ScriptedEventComponent);

	SERIALIZE_COMPONENT((struct ScriptComponent
	{
		PROPERTY(Name = Scripts) std::vector<WireGUID> scripts;

		CREATE_COMPONENT_GUID("{4FB7727F-BDBE-47E6-9C14-40ECBC5C7927}"_guid);
	}), ScriptComponent);

	SERIALIZE_COMPONENT((struct MonoScriptComponent
	{
		PROPERTY(Name = Script) std::string script;

		CREATE_COMPONENT_GUID("{CF0E9A06-FB14-4B56-BC9C-5557E808B829}"_guid);
	}), MonoScriptComponent);

	SERIALIZE_COMPONENT((struct CameraComponent
	{
		CameraComponent()
		{
			camera = CreateRef<Camera>(fieldOfView, 16.f / 9.f, nearPlane, farPlane);
		}

		PROPERTY(Name = Field of View) float fieldOfView = 60.f;
		PROPERTY(Name = Near plane) float nearPlane = 1.f;
		PROPERTY(Name = Far plane) float farPlane = 100000.f;
		PROPERTY(Name = Priority) uint32_t priority = 0;
		PROPERTY(Name = SmoothTime) float smoothTime = 0.040f; //Good Value :)

		Ref<Camera> camera;

		CREATE_COMPONENT_GUID("{9258BEEC-3A31-4CAB-AB1E-654524E1C398}"_guid);
	}), CameraComponent);

	SERIALIZE_COMPONENT((struct AnimatedCharacterComponent
	{
		PROPERTY(Name = Character, SpecialType = Asset, AssetType = AnimatedCharacter) AssetHandle animatedCharacter = Asset::Null();
		PROPERTY(Name = Cast Shadows) bool castShadows = true;

		uint32_t currentAnimation = 0;
		float currentStartTime = 0.f;
		bool isLooping = true;

		// Crossfading
		float crossfadeStartTime = 0.f;
		bool isCrossFading = false;
		bool shouldCrossfade = false;

		uint32_t crossfadeFrom = 0;
		uint32_t crossfadeTo = 0;

		// Override
		std::unordered_map<std::string, gem::mat4> boneOverrides;

		// Test
		Ref<AnimationStateMachine> characterStateMachine;

		CREATE_COMPONENT_GUID("{37333031-9816-4DDE-BFEA-5E83E32754D1}"_guid);
	}), AnimatedCharacterComponent);

	SERIALIZE_COMPONENT((struct EntityDataComponent
	{
		float timeSinceCreation = 0.f;
		bool isHighlighted = false;

		CREATE_COMPONENT_GUID("{A6789316-2D82-46FC-8138-B7BCBB9EA5B8}"_guid);
	}), EntityDataComponent);

	SERIALIZE_COMPONENT((struct TextRendererComponent
	{
		PROPERTY(Name = Text) std::string text = "Text";
		PROPERTY(Name = Font, SpecialType = Asset, AssetType = Font) AssetHandle fontHandle;
		PROPERTY(Name = Max Width) float maxWidth = 100.f;

		CREATE_COMPONENT_GUID("{8AAA0646-40D2-47E6-B83F-72EA26BD8C01}"_guid);
	}), TextRendererComponent);

	SERIALIZE_COMPONENT((struct VideoPlayerComponent
	{
		PROPERTY(Name = Video, SpecialType = Asset, AssetType = Video) AssetHandle videoHandle = Volt::Asset::Null();

		AssetHandle lastVideoHandle = Volt::Asset::Null();

		CREATE_COMPONENT_GUID("{15F85B2A-F8B2-48E1-8841-3BA946FFD172}"_guid);
	}), VideoPlayerComponent);

	SERIALIZE_COMPONENT((struct AudioListenerComponent
	{
		CREATE_COMPONENT_GUID("{3272F31A-F0C7-463E-A790-80A5B42D64BB}"_guid);
	}), AudioListenerComponent);

	SERIALIZE_ENUM((enum class EventTriggerCondition : uint32_t
	{
		None = 0,
		Start,
		Destroy,
		TiggerEnter,
		TriggerExit,
		CollisionEnter,
		CollisionExit

	}), EventTriggerCondition);

	SERIALIZE_COMPONENT((struct AudioEventEmitterComponent
	{
		PROPERTY(Name = PlayEvent, SpecialType = Enum) EventTriggerCondition playTriggerCondition = EventTriggerCondition::None;
		PROPERTY(Name = StopEvent, SpecialType = Enum) EventTriggerCondition stopTriggerCondition = EventTriggerCondition::None;
		PROPERTY(Name = Event) std::string eventPath = "";
		PROPERTY(Name = Trigger Once) bool isTriggerOnce = false;

		CREATE_COMPONENT_GUID("{9CF85ECF-4105-47D7-AB14-14259B77DDE2}"_guid);
	}), AudioEventEmitterComponent);

	SERIALIZE_COMPONENT((struct AudioParameterTriggerComponent
	{
		PROPERTY(Name = Entity ID) UINT entityID = 0;
		PROPERTY(Name = Trigger, SpecialType = Enum) EventTriggerCondition triggerCondition = EventTriggerCondition::None;
		PROPERTY(Name = Parameter Path) std::string eventPath = "";
		PROPERTY(Name = Value) float parameterValue;
		PROPERTY(Name = Trigger Once) bool isTriggerOnce = false;

		CREATE_COMPONENT_GUID("{63ECE9EF-2DC6-405F-ABAD-3B364DF08BE0}"_guid);
	}), AudioParameterTriggerComponent);

	SERIALIZE_COMPONENT((struct AudioSourceComponent
	{
		CREATE_COMPONENT_GUID("{251C7333-E3A0-4189-994A-10EA95C6FC34}"_guid);
	}), AudioSourceComponent);

	SERIALIZE_COMPONENT((struct AnimationControllerComponent
	{
		//PROPERTY(Name = AnimationTree) AssetHandle handle = Asset::Null();
		PROPERTY(Name = AnimationTreeFile) std::string animationTreeFile;
		Ref<AnimationTreeController> AnimTreeControl;

		CREATE_COMPONENT_GUID("{36D3CFA2-538E-4036-BB28-2B672F294478}"_guid);
	}), AnimationControllerComponent);

}