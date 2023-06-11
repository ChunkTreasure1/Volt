#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/Particles/Particle.h"
#include "Volt/BehaviorTree/BehaviorTree.hpp"

#include <Wire/Serialization.h>
#include <glm/glm.hpp>
#include <string>

namespace GraphKey
{
	class Graph;
}

namespace Volt
{
	class AnimationController;

	SERIALIZE_COMPONENT((struct TagComponent
	{
		TagComponent(const std::string& aTag) : tag(aTag) {}
		TagComponent() = default;

		PROPERTY(Name = Tag) std::string tag;

		CREATE_COMPONENT_GUID("{282FA5FB-6A77-47DB-8340-3D34F1A1FBBD}"_guid);
	}), TagComponent);

	SERIALIZE_COMPONENT((struct TransformComponent
	{
		PROPERTY(Name = Position) glm::vec3 position;
		PROPERTY(Name = Rotation) glm::quat rotation;
		PROPERTY(Name = Scale) glm::vec3 scale;

		PROPERTY(Visible = false) bool visible = true;
		PROPERTY(Visible = false) bool locked = false;

		inline const glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.f), position) *
				glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.f), scale);
		}

		inline const glm::vec3 GetForward() const
		{
			return glm::rotate(rotation, glm::vec3{ 0.f, 0.f, 1.f });
		}

		inline const glm::vec3 GetRight() const
		{
			return glm::rotate(rotation, glm::vec3{ 1.f, 0.f, 0.f });
		}

		inline const glm::vec3 GetUp() const
		{
			return glm::rotate(rotation, glm::vec3{ 0.f, 1.f, 0.f });
		}

		CREATE_COMPONENT_GUID("{E1B8016B-1CAA-4782-927E-C17C29B25893}"_guid);
	}), TransformComponent);

	SERIALIZE_COMPONENT((struct RelationshipComponent
	{
		PROPERTY(Name = Children) std::vector<Wire::EntityId> Children;
		PROPERTY(Name = Parent) Wire::EntityId Parent = 0;
		CREATE_COMPONENT_GUID("{4A5FEDD2-4D0B-4696-A9E6-DCDFFB25B32C}"_guid);

	}), RelationshipComponent);

	SERIALIZE_COMPONENT((struct PrefabComponent
	{
		PROPERTY(Name = PrefabAsset, Visible = false) AssetHandle prefabAsset = Asset::Null();
		PROPERTY(Name = PrefabEntity, Visible = false) Wire::EntityId prefabEntity = Wire::NullID;
		PROPERTY(Name = Version, Visible = false) uint32_t version = 0;

		bool isDirty = false;

		CREATE_COMPONENT_GUID("{B8A83ACF-F1CA-4C9F-8D1E-408B5BB388D2}"_guid);
	}), PrefabComponent);

	SERIALIZE_COMPONENT((struct ParticleEmitterComponent
	{
		PROPERTY(Name = Preset, SpecialType = Asset, AssetType = ParticlePreset) AssetHandle preset;
		AssetHandle currentPreset;

		float burstTimer = 0;

		bool isLooping = false;
		float internalTimer = 0;
		float emissionTimer = 0;
		int maxSpawnAmount = 0;

		CREATE_COMPONENT_GUID("{E31271AB-47C7-4D6A-91E8-4B1A62B20D66}"_guid);
	}), ParticleEmitterComponent);

	SERIALIZE_COMPONENT((struct MeshComponent
	{
		PROPERTY(Name = Mesh, SpecialType = Asset, AssetType = Mesh) AssetHandle handle = Asset::Null();
		PROPERTY(Name = Material, SpecialType = Asset, AssetType = Material) AssetHandle overrideMaterial = Asset::Null();

		int32_t subMaterialIndex = -1;

		CREATE_COMPONENT_GUID("{45D008BE-65C9-4D6F-A0C6-377F7B384E47}"_guid)
	}), MeshComponent);

	SERIALIZE_COMPONENT((struct DecalComponent
	{
		PROPERTY(Name = Material, SpecialType = Asset, AssetType = Material) AssetHandle materialHandle = Asset::Null();

		CREATE_COMPONENT_GUID("{09FA1C73-D508-4ADA-A101-A63703E91345}"_guid)
	}), DecalComponent);

	struct MonoScriptEntry
	{
		MonoScriptEntry(std::string& aName, UUID& aId) : name(aName), id(aId) {}

		std::string& name;
		UUID& id;
	};

	SERIALIZE_COMPONENT((struct MonoScriptComponent
	{
		PROPERTY(Visible = false) std::vector<std::string> scriptNames;
		PROPERTY(Visible = false) std::vector<AssetHandle> scriptIds;

		CREATE_COMPONENT_GUID("{CF0E9A06-FB14-4B56-BC9C-5557E808B829}"_guid);
	}), MonoScriptComponent);

	SERIALIZE_COMPONENT((struct CameraComponent
	{
		CameraComponent()
		{
			camera = CreateRef<Camera>(fieldOfView, 16.f / 9.f, nearPlane, farPlane);
		}

		PROPERTY(Name = Aperture) float aperture = 16.f;
		PROPERTY(Name = Shutter Speed) float shutterSpeed = 1.f / 100.f;
		PROPERTY(Name = ISO) float iso = 100.f;

		PROPERTY(Name = Field of View) float fieldOfView = 60.f;
		PROPERTY(Name = Near plane) float nearPlane = 1.f;
		PROPERTY(Name = Far plane) float farPlane = 100000.f;
		PROPERTY(Name = Priority) uint32_t priority = 0;

		Ref<Camera> camera;

		CREATE_COMPONENT_GUID("{9258BEEC-3A31-4CAB-AB1E-654524E1C398}"_guid);
	}), CameraComponent);

	SERIALIZE_COMPONENT((struct AnimatedCharacterComponent
	{
		PROPERTY(Name = Character, SpecialType = Asset, AssetType = AnimatedCharacter) AssetHandle animatedCharacter = Asset::Null();
		PROPERTY(Name = Cast Shadows) bool castShadows = true;

		int32_t selectedFrame = -1;
		uint32_t currentAnimation = 0;
		float currentStartTime = 0.f;

		std::unordered_map<UUID, std::vector<Entity>> attachedEntities;

		bool isLooping = true;
		bool isPlaying = false;

		CREATE_COMPONENT_GUID("{37333031-9816-4DDE-BFEA-5E83E32754D1}"_guid);
	}), AnimatedCharacterComponent);

	SERIALIZE_COMPONENT((struct EntityDataComponent
	{
		PROPERTY(Visible = false) uint32_t layerId = 0;

		float timeSinceCreation = 0.f;
		float randomValue = 0.f;
		bool isHighlighted = false;

		CREATE_COMPONENT_GUID("{A6789316-2D82-46FC-8138-B7BCBB9EA5B8}"_guid);
	}), EntityDataComponent);

	SERIALIZE_COMPONENT((struct TextRendererComponent
	{
		PROPERTY(Name = Text) std::string text = "Text";
		PROPERTY(Name = Font, SpecialType = Asset, AssetType = Font) AssetHandle fontHandle;
		PROPERTY(Name = Max Width) float maxWidth = 100.f;
		PROPERTY(Name = Color, SpecialType = Color) glm::vec4 color{ 1.f };

		CREATE_COMPONENT_GUID("{8AAA0646-40D2-47E6-B83F-72EA26BD8C01}"_guid);
	}), TextRendererComponent);

	SERIALIZE_COMPONENT((struct VideoPlayerComponent
	{
		PROPERTY(Name = Video, SpecialType = Asset, AssetType = Video) AssetHandle videoHandle = Volt::Asset::Null();

		AssetHandle lastVideoHandle = Volt::Asset::Null();

		CREATE_COMPONENT_GUID("{15F85B2A-F8B2-48E1-8841-3BA946FFD172}"_guid);
	}), VideoPlayerComponent);

	SERIALIZE_COMPONENT((struct SpriteComponent
	{
		PROPERTY(Name = Material, SpecialType = Asset, AssetType = Material) AssetHandle materialHandle = Volt::Asset::Null();

		CREATE_COMPONENT_GUID("{FDB47734-1B69-4558-B460-0975365DB400}"_guid);
	}), SpriteComponent);

	SERIALIZE_COMPONENT((struct AnimationControllerComponent
	{
		PROPERTY(Name = Animation Graph, SpecialType = Asset, AssetType = AnimationGraph) AssetHandle animationGraph = Volt::Asset::Null();
		PROPERTY(Name = Material, SpecialType = Asset, AssetType = Material) AssetHandle overrideMaterial = Volt::Asset::Null();
		PROPERTY(Name = Apply Root Motion) bool applyRootMotion = false;

		Ref<AnimationController> controller;

		CREATE_COMPONENT_GUID("{36D3CFA2-538E-4036-BB28-2B672F294478}"_guid);
	}), AnimationControllerComponent);

	SERIALIZE_COMPONENT((struct VisualScriptingComponent
	{
		PROPERTY(Name = GraphState, Visible = false) std::string graphState;
		Ref<GraphKey::Graph> graph;

		CREATE_COMPONENT_GUID("{1BC207FE-D06C-41C2-83F4-E153F3A75770}"_guid);
	}), VisualScriptingComponent);

	SERIALIZE_COMPONENT((struct BehaviorTreeComponent
	{
		PROPERTY(Name = Tree Handle, SpecialType = Asset, AssetType = BehaviorTree::Tree) AssetHandle treeHandle = Volt::Asset::Null();
		Ref<BehaviorTree::Tree> tree;

		CREATE_COMPONENT_GUID("{D1B1C21E-FA8C-49E9-80CB-B36233FB0575}"_guid);
	}), BehaviorTreeComponent);

	SERIALIZE_COMPONENT((struct VertexPaintedComponent
	{
		PROPERTY(Visible = false, Serializable =  false) AssetHandle meshHandle;
		PROPERTY(Visible = false, Serializable = false) std::vector<glm::vec4> vertecies;
		CREATE_COMPONENT_GUID("{480B6514-05CB-4532-A366-B5DFD419E310}"_guid);
	}), VertexPaintedComponent);

	SERIALIZE_COMPONENT((struct PostProcessingStackComponent
	{
		PROPERTY(Name = Post Processing Stack, SpecialType = Asset, AssetType = PostProcessingStack) AssetHandle postProcessingStack = Volt::Asset::Null();

		CREATE_COMPONENT_GUID("{09340235-CDA0-496E-BEB5-A2F38BCE0033}"_guid);
	}), PostProcessingStackComponent);
}
