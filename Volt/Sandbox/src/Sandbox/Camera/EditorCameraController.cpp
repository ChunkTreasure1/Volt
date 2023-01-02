#include "sbpch.h"
#include "EditorCameraController.h"

#include "Volt/Core/Application.h"
#include "Volt/Rendering/Camera/Camera.h"

#include "Volt/Events/ApplicationEvent.h"

#include "Volt/Input/Input.h"
#include "Volt/Input/MouseButtonCodes.h"
#include "Volt/Input/KeyCodes.h"

#include "Volt/Utility/UIUtility.h"

#include <GLFW/glfw3.h>

EditorCameraController::EditorCameraController(float fov, float nearPlane, float farPlane)
	: m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane)
{
	const gem::vec3 startPosition = { 500.f, 500.f, 500.f };
	m_focalDistance = gem::distance(startPosition, m_focalPoint);
	m_rotation = { 45.f, 135.f, 0.f };
	m_position = startPosition;

	m_camera = CreateRef<Volt::Camera>(fov, 16.f / 9.f, nearPlane, farPlane);
	m_camera->SetRotation(m_rotation);
	m_position = CalculatePosition();
	m_camera->SetPosition(m_position);
}

EditorCameraController::~EditorCameraController()
{}

void EditorCameraController::SetIsControllable(bool hovered)
{
	m_isViewportHovered = hovered;
}

void EditorCameraController::ForceLooseControl()
{
	m_isControllable = false;
}

void EditorCameraController::Focus(const gem::vec3& focusPoint)
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

void EditorCameraController::UpdateProjection(uint32_t width, uint32_t height)
{
	float aspectRatio = (float)width / (float)height;
	m_camera->SetPerspectiveProjection(m_fov, aspectRatio, m_nearPlane, m_farPlane);
}

void EditorCameraController::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(EditorCameraController::OnUpdateEvent));
	dispatcher.Dispatch<Volt::MouseScrolledEvent>(VT_BIND_EVENT_FN(EditorCameraController::OnMouseScrolled));
	dispatcher.Dispatch<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(EditorCameraController::OnMousePressedEvent));
	dispatcher.Dispatch<Volt::MouseMovedViewportEvent>([&](Volt::MouseMovedViewportEvent& e)
		{
			m_ViewPortMousePos = { e.GetX(), e.GetY() };

			return false;
		});

	dispatcher.Dispatch<Volt::ViewportResizeEvent>([&](Volt::ViewportResizeEvent& e)
		{
			m_ViewPortSize = { (float)e.GetWidth(), (float)e.GetHeight() };
			return false;
		});
}
bool EditorCameraController::OnMousePressedEvent(Volt::MouseButtonPressedEvent& e)
{
	if (m_isViewportHovered)
	{
		const auto [x, y] = Volt::Input::GetMousePosition();
		m_lastMousePosition = { x, y };
	}

	return false;
}

bool EditorCameraController::OnMouseReleasedEvent(Volt::MouseButtonReleasedEvent& e)
{
	
	return false;
}

void EditorCameraController::DisableMouse()
{
	Volt::Application::Get().GetWindow().ShowCursor(false);
	UI::SetInputEnabled(false);
}

void EditorCameraController::EnableMouse()
{
	Volt::Application::Get().GetWindow().ShowCursor(true);
	UI::SetInputEnabled(true);
}

void EditorCameraController::ArcBall(const gem::vec2& deltaPos)
{
	const float yawSign = m_camera->GetUp().y < 0.f ? -1.f : 1.f;
	m_yawDelta += yawSign * deltaPos.x * m_sensitivity;
	m_pitchDelta += deltaPos.y * m_sensitivity;
}

void EditorCameraController::ArcZoom(float deltaPos)
{
	float distance = m_focalDistance * 0.2f;
	distance = gem::max(distance, 0.0f);
	float speed = distance * distance;
	speed = gem::min(speed, 10.f ); // max speed = 50

	m_focalDistance -= deltaPos * speed;
	m_position = m_focalPoint - m_camera->GetForward() * m_focalDistance;
	if (m_focalDistance < 1.f)
	{
		m_focalPoint += m_camera->GetForward() * m_focalDistance;
		m_focalDistance = 1.f;
	}

	m_positionDelta += deltaPos * speed * m_camera->GetForward();
}

const gem::vec3 EditorCameraController::CalculatePosition() const
{
	return m_focalPoint - m_camera->GetForward() * m_focalDistance + m_positionDelta;
}

gem::vec3 EditorCameraController::GetScreenToRayDir()
{
	return m_camera->ScreenToWorldCoords(m_ViewPortMousePos, m_ViewPortSize);
}

bool EditorCameraController::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	const gem::vec2 mousePos = { Volt::Input::GetMouseX(), Volt::Input::GetMouseY() };
	const gem::vec2 deltaPos = (mousePos - m_lastMousePosition);

	m_pitchDelta = 0.f;
	m_yawDelta = 0.f;
	m_positionDelta = 0.f;

	if (m_isViewportHovered)
	{
		m_isControllable = true;
	}

	if (Volt::Input::IsMouseButtonPressed(VT_MOUSE_BUTTON_RIGHT) && !Volt::Input::IsKeyDown(VT_KEY_LEFT_ALT) && m_isControllable)
	{
		m_isControllable = true;
		m_cameraMode = Mode::Fly;

		DisableMouse();

		const float yawSign = m_camera->GetUp().y < 0 ? -1.0f : 1.0f;

		if (Volt::Input::IsKeyDown(VT_KEY_W))
		{
			m_positionDelta += m_translationSpeed * m_camera->GetForward() * e.GetTimestep();
		}
		if (Volt::Input::IsKeyDown(VT_KEY_S))
		{
			m_positionDelta -= m_translationSpeed * m_camera->GetForward() * e.GetTimestep();
		}
		if (Volt::Input::IsKeyDown(VT_KEY_A))
		{
			m_positionDelta -= m_translationSpeed * m_camera->GetRight() * e.GetTimestep();
		}
		if (Volt::Input::IsKeyDown(VT_KEY_D))
		{
			m_positionDelta += m_translationSpeed * m_camera->GetRight() * e.GetTimestep();
		}

		if (Volt::Input::IsKeyDown(VT_KEY_E))
		{
			m_positionDelta += m_translationSpeed * e.GetTimestep() * gem::vec3{ 0.f, yawSign, 0.f };
		}
		if (Volt::Input::IsKeyDown(VT_KEY_Q))
		{
			m_positionDelta -= m_translationSpeed * e.GetTimestep() * gem::vec3{ 0.f, yawSign, 0.f };
		}

		constexpr float maxSpeed = 30.f;

		m_yawDelta += gem::clamp(yawSign * deltaPos.x * m_sensitivity, -maxSpeed, maxSpeed);
		m_pitchDelta += gem::clamp(deltaPos.y * m_sensitivity, -maxSpeed, maxSpeed);

		m_focalDistance = gem::distance(m_focalPoint, m_position);
		m_focalPoint = m_position + m_camera->GetForward() * m_focalDistance;
	}
	else if (Volt::Input::IsKeyDown(VT_KEY_LEFT_ALT) && m_isControllable)
	{
		m_cameraMode = Mode::ArcBall;

		if (Volt::Input::IsMouseButtonPressed(VT_MOUSE_BUTTON_LEFT))
		{
			DisableMouse();
			ArcBall(deltaPos);
		}
		else if (Volt::Input::IsMouseButtonPressed(VT_MOUSE_BUTTON_MIDDLE))
		{
			DisableMouse();
			m_focalPoint += -1.f * m_camera->GetRight() * deltaPos.x;
			m_focalPoint += m_camera->GetUp() * deltaPos.y;
		}
		else if (Volt::Input::IsMouseButtonPressed(VT_MOUSE_BUTTON_RIGHT))
		{
			DisableMouse();
			ArcZoom(deltaPos.x + deltaPos.y);
		}
		else
		{
			EnableMouse();
		}
	}
	else
	{
		m_isControllable = false;
		EnableMouse();
	}

	m_position += m_positionDelta;
	m_rotation += { m_pitchDelta, m_yawDelta, 0.f };

	if (m_cameraMode == Mode::Fly)
	{
		m_rotation.x = gem::clamp(m_rotation.x, -89.f, 89.f);
	}

	m_camera->SetPosition(m_position);
	m_camera->SetRotation(gem::radians(m_rotation));

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
	if (Volt::Input::IsMouseButtonPressed(VT_MOUSE_BUTTON_RIGHT))
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
