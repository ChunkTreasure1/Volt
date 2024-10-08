#include "vtpch.h"

#include "Volt/Components/CoreComponents.h"
#include "Volt/Components/RenderingComponents.h"
#include "Volt/Rendering/Camera/Camera.h"

#include "Volt/Animation/MotionWeaver.h"

#include <EntitySystem/Scripting/CommonComponent.h>
#include <EntitySystem/Scripting/ECSBuilder.h>
#include <EntitySystem/Scripting/ECSSystemRegistry.h>

namespace Volt
{
	using CommonEntity = ECS::Access
		::Write<CommonComponent>
		::As<ECS::Type::Entity>;

	void CommonSystem(CommonEntity entity, float deltaTime)
	{
		entity.GetComponent<CommonComponent>().timeSinceCreation += deltaTime;
	}

	using CameraEntity = ECS::Access
		::Write<CameraComponent>
		::Read<TransformComponent>
		::As<ECS::Type::Entity>;

	void CameraSystem(CameraEntity entity, float deltaTime)
	{
		const auto& transform = entity.GetComponent<TransformComponent>();

		if (!transform.visible)
		{
			return;
		}

		// #TODO_Ivar: This should be updated to include correct aspect ration and to use correct world position/rotation.
		auto& cameraComponent = entity.GetComponent<CameraComponent>();
		cameraComponent.camera->SetPerspectiveProjection(cameraComponent.fieldOfView, 16.f / 9.f, cameraComponent.nearPlane, cameraComponent.farPlane);
		cameraComponent.camera->SetPosition(transform.position);
		cameraComponent.camera->SetRotation(glm::eulerAngles(transform.rotation));
	}

	using MotionWeaveEntity = ECS::Access
		::Write<MotionWeaveComponent>
		::With<MeshComponent>
		::As<ECS::Type::Entity>;

	void MotionWeaveSystem(MotionWeaveEntity entity, float deltaTime)
	{
		auto& weaveComponent = entity.GetComponent<MotionWeaveComponent>();
		if (weaveComponent.MotionWeaver)
		{
			weaveComponent.MotionWeaver->Update(deltaTime);
		}
	}

	void RegisterModule(ECSBuilder& builder)
	{
		builder.GetGameLoop(GameLoop::Variable).RegisterSystem(CommonSystem);
		builder.GetGameLoop(GameLoop::Variable).RegisterSystem(CameraSystem);
		builder.GetGameLoop(GameLoop::Variable).RegisterSystem(MotionWeaveSystem);
	}

	VT_REGISTER_ECS_MODULE(RegisterModule);
}
