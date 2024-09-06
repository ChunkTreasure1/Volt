#pragma once

#include <EventSystem/EventListener.h>

#include <glm/glm.hpp>

namespace Volt
{
	class Camera;
	class Event;
	class AppUpdateEvent;
	class MouseScrolledEvent;
	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
}

class EditorCameraController : public Volt::EventListener
{
public:
	EditorCameraController(float fov, float nearPlane, float farPlane);
	~EditorCameraController();

	void UpdateProjection(uint32_t width, uint32_t height);

	void SetTranslationSpeed(float speed) { m_translationSpeed = speed; }
	float GetTranslationSpeed() { return m_translationSpeed; }

	void Focus(const glm::vec3& focusPoint);
	
	void SetControllable(bool enabled);
	void ForceDisable();

	inline Ref<Volt::Camera> GetCamera() { return m_camera; }

private:
	enum class Mode
	{
		Fly,
		ArcBall
	};

	void RegisterEventListeners();

	bool OnUpdateEvent(Volt::AppUpdateEvent& e);
	bool OnMouseScrolled(Volt::MouseScrolledEvent& e);
	bool OnMousePressedEvent(Volt::MouseButtonPressedEvent& e);
	bool OnMouseReleasedEvent(Volt::MouseButtonReleasedEvent& e);

	void DisableMouse();
	void EnableMouse();

	void ArcBall(const glm::vec2& deltaPos);
	void ArcZoom(float deltaPos);

	const glm::vec3 CalculatePosition() const;

	Ref<Volt::Camera> m_camera;

	glm::vec2 m_lastMousePosition = { 0.f, 0.f };
	glm::vec3 m_position = { 0.f, 0.f, 0.f };
	glm::vec3 m_rotation = { 0.f, 0.f, 0.f };

	glm::vec3 m_positionDelta = { 0.f, 0.f, 0.f };
	float m_pitchDelta = 0.f;
	float m_yawDelta = 0.f;

	glm::vec3 m_focalPoint = { 0.f, 0.f, 0.f };
	float m_focalDistance = 100.f;
	float m_minFocalDistance = 10000.f;

	float m_fov = 45.f;
	float m_nearPlane = 0.1f;
	float m_farPlane = 10000.f;

	float m_translationSpeed = 100.f;
	float m_scrollTranslationSpeed = 100.f;

	float m_maxTranslationSpeed = 100000.f;
	float m_sensitivity = 0.12f;

	bool m_isEnabled = false;
	bool m_isControllable = false;

	Mode m_cameraMode = Mode::Fly;
};
