#pragma once

#include "Volt/Scene/Entity.h"
#include "Volt/Asset/Rendering/MaterialTable.h"

#include <EntitySystem/ComponentRegistry.h>
#include <EntitySystem/Scripting/ECSAccessBuilder.h>

#include <AssetSystem/Asset.h>

namespace Volt
{
	class Camera;
	class AnimationController;
	class ScenePrimitiveData;
	class MotionWeaver;

	struct MeshComponent
	{
		using MeshEntity = ECS::Access
			::Write<MeshComponent>
			::Read<IDComponent>
			::As<ECS::Type::Entity>;

		AssetHandle handle = Asset::Null();
		Vector<AssetHandle> materials;

		[[nodiscard]] inline const AssetHandle& GetHandle() const { return handle; }

		static void ReflectType(TypeDesc<MeshComponent>& reflect)
		{
			reflect.SetGUID("{45D008BE-65C9-4D6F-A0C6-377F7B384E47}"_guid);
			reflect.SetLabel("Mesh Component");
			reflect.AddMember(&MeshComponent::handle, "handle", "Mesh", "", Asset::Null(), AssetTypes::Mesh);
			reflect.AddMember(&MeshComponent::materials, "materials", "Materials", "", Vector<AssetHandle>{}, AssetTypes::Material);
			reflect.SetOnMemberChangedCallback(&MeshComponent::OnMemberChanged);
			reflect.SetOnComponentCopiedCallback(&MeshComponent::OnComponentCopied);
			reflect.SetOnDestroyCallback(&MeshComponent::OnDestroy);
			reflect.SetOnCreateCallback(&MeshComponent::OnCreate);
			reflect.SetOnTransformChangedCallback(&MeshComponent::OnTransformChanged);
		}

		REGISTER_COMPONENT(MeshComponent);

		static void OnMemberChanged(MeshEntity entity);

	private:
		static void OnCreate(MeshEntity entity);
		static void OnDestroy(MeshEntity entity);
		static void OnTransformChanged(MeshEntity entity);
		static void OnComponentCopied(MeshEntity entity);

		Ref<ScenePrimitiveData> m_scenePrimitiveData;
	};

	struct CameraComponent
	{
		float fieldOfView = 60.f;
		float nearPlane = 1.f;
		float farPlane = 100'000.f;
		uint32_t priority = 0;

		Ref<Camera> camera;

		static void ReflectType(TypeDesc<CameraComponent>& reflect)
		{
			reflect.SetGUID("{9258BEEC-3A31-4CAB-AB1E-654524E1C398}"_guid);
			reflect.SetLabel("Camera Component");
			reflect.AddMember(&CameraComponent::fieldOfView, "fieldOfView", "Field Of View", "", 60.f);
			reflect.AddMember(&CameraComponent::nearPlane, "nearPlane", "Near Plane", "", 1.f);
			reflect.AddMember(&CameraComponent::farPlane, "farPlane", "Far Plane", "", 100'000.f);
			reflect.AddMember(&CameraComponent::priority, "priority", "Priority", "", 0);
			reflect.SetOnCreateCallback(&CameraComponent::OnCreate);
		}

		REGISTER_COMPONENT(CameraComponent);

	private:
		using CameraEntity = ECS::Access
			::Write<CameraComponent>
			::As<ECS::Type::Entity>;

		static void OnCreate(CameraEntity entity);
	};

	struct AnimatedCharacterComponent
	{
		AssetHandle animatedCharacter = Asset::Null();

		int32_t selectedFrame = -1;
		uint32_t currentAnimation = 0;
		float currentStartTime = 0.f;

		std::unordered_map<UUID64, Vector<Entity>> attachedEntities;

		bool isLooping = true;
		bool isPlaying = false;

		static void ReflectType(TypeDesc<AnimatedCharacterComponent>& reflect)
		{
			reflect.SetGUID("{37333031-9816-4DDE-BFEA-5E83E32754D1}"_guid);
			reflect.SetLabel("Animated Character Component");
			reflect.AddMember(&AnimatedCharacterComponent::animatedCharacter, "animatedCharacter", "Character", "", Asset::Null(), AssetTypes::AnimatedCharacter);
		}

		REGISTER_COMPONENT(AnimatedCharacterComponent);
	};

	struct TextRendererComponent
	{
		std::string text = "Text";
		AssetHandle font = Asset::Null();
		float maxWidth = 100.f;
		glm::vec4 color = { 1.f };

		static void ReflectType(TypeDesc<TextRendererComponent>& reflect)
		{
			reflect.SetGUID("{8AAA0646-40D2-47E6-B83F-72EA26BD8C01}"_guid);
			reflect.SetLabel("Text Renderer Component");
			reflect.AddMember(&TextRendererComponent::text, "text", "Text", "", std::string("Text"));
			reflect.AddMember(&TextRendererComponent::font, "font", "Font", "", Asset::Null(), AssetTypes::Font);
			reflect.AddMember(&TextRendererComponent::maxWidth, "maxWidth", "Max Width", "", 100.f);
			reflect.AddMember(&TextRendererComponent::color, "color", "Color", "", glm::vec4{ 1.f }, ComponentMemberFlag::Color4);
		}

		REGISTER_COMPONENT(TextRendererComponent);
	};

	struct SpriteComponent
	{
		AssetHandle materialHandle = Asset::Null();

		static void ReflectType(TypeDesc<SpriteComponent>& reflect)
		{
			reflect.SetGUID("{FDB47734-1B69-4558-B460-0975365DB400}"_guid);
			reflect.SetLabel("Sprite Component");
			reflect.AddMember(&SpriteComponent::materialHandle, "materialHandle", "Material", "", Asset::Null(), AssetTypes::Material);
		}

		REGISTER_COMPONENT(SpriteComponent);
	};

	struct AnimationControllerComponent
	{
		AssetHandle material = Asset::Null();
		AssetHandle skin = Asset::Null();
		bool applyRootMotion = false;

		Ref<AnimationController> controller;

		static void ReflectType(TypeDesc<AnimationControllerComponent>& reflect)
		{
			reflect.SetGUID("{36D3CFA2-538E-4036-BB28-2B672F294478}"_guid);
			reflect.SetLabel("Animation Controller Component");
			reflect.AddMember(&AnimationControllerComponent::material, "material", "Material", "", Asset::Null(), AssetTypes::Material);
			reflect.AddMember(&AnimationControllerComponent::skin, "skin", "Skin", "", Asset::Null(), AssetTypes::Mesh);
			reflect.AddMember(&AnimationControllerComponent::applyRootMotion, "applyRootMotion", "Apply Root Motion", "", false);
		}

		REGISTER_COMPONENT(AnimationControllerComponent);
	};

	struct MotionWeaveComponent
	{
		AssetHandle motionWeaveDatabase = Asset::Null();

		Ref<MotionWeaver> MotionWeaver;
		Vector<UUID64> renderObjectIds;

		static void ReflectType(TypeDesc<MotionWeaveComponent>& reflect)
		{
			reflect.SetGUID("{5D3B2C0D-5457-43D8-9623-98730E1556F4}"_guid);
			reflect.SetLabel("Motion Weave Component");
			reflect.AddMember(&MotionWeaveComponent::motionWeaveDatabase, "motionGraph", "Motion Graph", "", Asset::Null(), AssetTypes::MotionWeave);
			reflect.SetOnStartCallback(&MotionWeaveComponent::OnStart);
		}

		REGISTER_COMPONENT(MotionWeaveComponent);

	private:
		using WeaveEntity = ECS::Access
			::Write<MotionWeaveComponent>
			::Read<MeshComponent>
			::Read<IDComponent>
			::As<ECS::Type::Entity>;

		static void OnStart(WeaveEntity entity);
	};

	struct VertexPaintedComponent
	{
		AssetHandle meshHandle = Asset::Null();
		Vector<uint32_t> vertexColors;

		static void ReflectType(TypeDesc<VertexPaintedComponent>& reflect)
		{
			reflect.SetGUID("{480B6514-05CB-4532-A366-B5DFD419E310}"_guid);
			reflect.SetLabel("Vertex Painted Component");
		}

		REGISTER_COMPONENT(VertexPaintedComponent);
	};

	struct PostProcessingStackComponent
	{
		AssetHandle postProcessingStack = Asset::Null();

		static void ReflectType(TypeDesc<PostProcessingStackComponent>& reflect)
		{
			reflect.SetGUID("{09340235-CDA0-496E-BEB5-A2F38BCE0033}"_guid);
			reflect.SetLabel("Post Processing Stack Component");
			reflect.AddMember(&PostProcessingStackComponent::postProcessingStack, "postProcessingStack", "Post Processing Stack", "", Asset::Null(), AssetTypes::PostProcessingStack);
		}

		REGISTER_COMPONENT(PostProcessingStackComponent);
	};

	struct DecalComponent
	{
		AssetHandle decalMaterial = Asset::Null();

		static void ReflectType(TypeDesc<DecalComponent>& reflect)
		{
			reflect.SetGUID("{09FA1C73-D508-4ADA-A101-A63703E91345}"_guid);
			reflect.SetLabel("Decal Component");
			reflect.AddMember(&DecalComponent::decalMaterial, "decalMaterial", "Material", "", Asset::Null(), AssetTypes::Material);
		}

		REGISTER_COMPONENT(DecalComponent);
	};

	struct ParticleEmitterComponent
	{
		AssetHandle preset = Asset::Null();
		AssetHandle currentPreset = Asset::Null();

		float burstTimer = 0;

		bool isLooping = false;
		float internalTimer = 0;
		float emissionTimer = 0;
		int maxSpawnAmount = 0;

		static void ReflectType(TypeDesc<ParticleEmitterComponent>& reflect)
		{
			reflect.SetGUID("{E31271AB-47C7-4D6A-91E8-4B1A62B20D66}"_guid);
			reflect.SetLabel("Particle Emitter Component");
			reflect.AddMember(&ParticleEmitterComponent::preset, "preset", "Preset", "", Asset::Null(), AssetTypes::ParticlePreset);
		}

		REGISTER_COMPONENT(ParticleEmitterComponent);
	};
}
