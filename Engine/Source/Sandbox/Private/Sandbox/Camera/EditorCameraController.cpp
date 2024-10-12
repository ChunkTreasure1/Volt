#include "sbpch.h"
#include "Camera/EditorCameraController.h"

#include "Volt/Core/Application.h"
#include "Volt/Rendering/Camera/Camera.h"


#include <InputModule/Input.h>
#include <InputModule/InputCodes.h>
#include <InputModule/MouseButtonCodes.h>

#include "Volt/Utility/UIUtility.h"

#include <Volt/Events/ApplicationEvents.h>
#include <InputModule/Events/KeyboardEvents.h>
#include <InputModule/Events/MouseEvents.h>

#include <GLFW/glfw3.h>

EditorCameraController::EditorCameraController(float fov, float nearPlane, float farPlane)
	: m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane)
{
	RegisterEventListeners();

	const glm::vec3 startPosition = { 500.f, 500.f, 500.f };
	m_focalDistance = glm::distance(startPosition, m_focalPoint);
	m_rotation = { 45.f, 135.f, 0.f };
	m_position = startPosition;

	m_camera = CreateRef<Volt::Camera>(fov, 16.f / 9.f, nearPlane, farPlane);
	m_camera->SetRotation(glm::radians(m_rotation));
	m_position = CalculatePosition();
	m_camera->SetPosition(m_position);
}

EditorCameraController::~EditorCameraController()
{
}

void EditorCameraController::Focus(const glm::vec3& focusPoint)
{
	m_focalPoint = focusPoint;
	m_cameraMode = Mode::Fly;

	if (m_focalDistance > m_minFocalDistance)
	{
		m_focalDistance -= m_focalDistance - m_minFocalDistance;
	}
	m_position = m_focalPoint - m_camera->GetForward() * m_focalDistance;
	m_camera->SetPosition(m_focalPoint);
}

void EditorCameraController::SetControllable(bool controllable)
{
	m_isControllable = controllable;
}

void EditorCameraController::ForceDisable()
{
	m_isEnabled = false;
	EnableMouse();
}

void EditorCameraController::UpdateProjection(uint32_t width, uint32_t height)
{
	float aspectRatio = (float)width / (float)height;
	m_camera->SetPerspectiveProjection(m_fov, aspectRatio, m_nearPlane, m_farPlane);
}

bool EditorCameraController::OnMousePressedEvent(Volt::MouseButtonPressedEvent& e)
{
	m_lastMousePosition = Volt::Input::GetMousePosition();

	if (m_isControllable)
	{
		m_isEnabled = true;
	}

	return false;
}

bool EditorCameraController::OnMouseReleasedEvent(Volt::MouseButtonReleasedEvent& e)
{
	if (m_isControllable)
	{
		EnableMouse();
		m_isEnabled = false;
	}
	return false;
}

void EditorCameraController::DisableMouse()
{
	Volt::Input::ShowCursor(false);
	UI::SetInputEnabled(false);
}

void EditorCameraController::EnableMouse()
{
	Volt::Input::ShowCursor(true);
	UI::SetInputEnabled(true);
}

void EditorCameraController::ArcBall(const glm::vec2& deltaPos)
{
	const float yawSign = m_camera->GetUp().y < 0.f ? -1.f : 1.f;
	m_yawDelta += yawSign * deltaPos.x * m_sensitivity;
	m_pitchDelta += deltaPos.y * m_sensitivity;
}

void EditorCameraController::ArcZoom(float deltaPos)
{
	float distance = m_focalDistance * 0.2f;
	distance = glm::max(distance, 0.0f);
	float speed = distance * distance;
	speed = glm::min(speed, 10.f); // max speed = 50

	m_focalDistance -= deltaPos * speed;
	m_position = m_focalPoint - m_camera->GetForward() * m_focalDistance;
	if (m_focalDistance < 1.f)
	{
		m_focalPoint += m_camera->GetForward() * m_focalDistance;
		m_focalDistance = 1.f;
	}

	m_positionDelta += deltaPos * speed * m_camera->GetForward();
}

const glm::vec3 EditorCameraController::CalculatePosition() const
{
	return m_focalPoint - m_camera->GetForward() * m_focalDistance + m_positionDelta;
}

void EditorCameraController::RegisterEventListeners()
{
	auto isControllable = [this]() {  return m_isControllable; };
	auto isEnabled = [this]() {  return m_isEnabled; };

	RegisterListener<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(EditorCameraController::OnUpdateEvent), isEnabled);
	RegisterListener<Volt::MouseScrolledEvent>(VT_BIND_EVENT_FN(EditorCameraController::OnMouseScrolled), isEnabled);
	RegisterListener<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(EditorCameraController::OnMousePressedEvent), isControllable);
	RegisterListener<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(EditorCameraController::OnMouseReleasedEvent), isEnabled);
}

bool EditorCameraController::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	const glm::vec2 mousePos = { Volt::Input::GetMouseX(), Volt::Input::GetMouseY() };
	const glm::vec2 deltaPos = (mousePos - m_lastMousePosition);

	m_pitchDelta = 0.f;
	m_yawDelta = 0.f;
	m_positionDelta = 0.f;

	if (Volt::Input::IsMouseButtonDown(Volt::InputCode::Mouse_RB) && !Volt::Input::IsMouseButtonDown(Volt::InputCode::Alt))
	{
		m_cameraMode = Mode::Fly;

		DisableMouse();

		const float yawSign = m_camera->GetUp().y < 0 ? -1.0f : 1.0f;

		if (Volt::Input::IsKeyDown(Volt::InputCode::W))
		{
			m_positionDelta += m_translationSpeed * m_camera->GetForward() * e.GetTimestep();
		}
		if (Volt::Input::IsKeyDown(Volt::InputCode::S))
		{
			m_positionDelta -= m_translationSpeed * m_camera->GetForward() * e.GetTimestep();
		}
		if (Volt::Input::IsKeyDown(Volt::InputCode::A))
		{
			m_positionDelta -= m_translationSpeed * m_camera->GetRight() * e.GetTimestep();
		}
		if (Volt::Input::IsKeyDown(Volt::InputCode::D))
		{
			m_positionDelta += m_translationSpeed * m_camera->GetRight() * e.GetTimestep();
		}

		if (Volt::Input::IsKeyDown(Volt::InputCode::E))
		{
			m_positionDelta += m_translationSpeed * e.GetTimestep() * glm::vec3{ 0.f, yawSign, 0.f };
		}
		if (Volt::Input::IsKeyDown(Volt::InputCode::Q))
		{
			m_positionDelta -= m_translationSpeed * e.GetTimestep() * glm::vec3{ 0.f, yawSign, 0.f };
		}

		constexpr float maxSpeed = 30.f;

		m_yawDelta += glm::clamp(yawSign * deltaPos.x * m_sensitivity, -maxSpeed, maxSpeed);
		m_pitchDelta += glm::clamp(deltaPos.y * m_sensitivity, -maxSpeed, maxSpeed);

		m_focalDistance = glm::distance(m_focalPoint, m_position);
		m_focalPoint = m_position + m_camera->GetForward() * m_focalDistance;
	}
	else if (Volt::Input::IsKeyDown(Volt::InputCode::LeftAlt))
	{
		m_cameraMode = Mode::ArcBall;

		if (Volt::Input::IsMouseButtonDown(Volt::InputCode::Mouse_LB))
		{
			DisableMouse();
			ArcBall(deltaPos);
		}
		else if (Volt::Input::IsMouseButtonDown(Volt::InputCode::Mouse_MB))
		{
			DisableMouse();
			m_focalPoint += -1.f * m_camera->GetRight() * deltaPos.x;
			m_focalPoint += m_camera->GetUp() * deltaPos.y;
		}
		else if (Volt::Input::IsMouseButtonDown(Volt::InputCode::Mouse_RB))
		{
			DisableMouse();
			ArcZoom((deltaPos.x + deltaPos.y) * m_sensitivity);
		}
		else
		{
			EnableMouse();
		}
	}
	else
	{
		EnableMouse();
	}

	m_position += m_positionDelta;
	m_rotation += glm::vec3{ m_pitchDelta, m_yawDelta, 0.f };

	if (m_cameraMode == Mode::Fly)
	{
		m_rotation.x = glm::clamp(m_rotation.x, -89.f, 89.f);
	}

	m_camera->SetPosition(m_position);
	m_camera->SetRotation(glm::radians(m_rotation));

	if (m_cameraMode == Mode::ArcBall)
	{
		m_position = CalculatePosition();
		m_camera->SetPosition(m_position);
	}

	m_lastMousePosition = mousePos;

	return false;
}

bool EditorCameraController::OnMouseScrolled(Volt::MouseScrolledEvent& e)
{
	if (Volt::Input::IsMouseButtonDown(Volt::InputCode::Mouse_RB))
	{
		m_translationSpeed += e.GetYOffset() * m_scrollTranslationSpeed;
		m_translationSpeed = std::min(m_translationSpeed, m_maxTranslationSpeed);

		if (m_translationSpeed < 0.f)
		{
			m_translationSpeed = 0.f;
		}
	}

	return false;
}
