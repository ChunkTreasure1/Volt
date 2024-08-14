#pragma once

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

class EditorCameraController
{
public:
	EditorCameraController(float fov, float nearPlane, float farPlane);
	~EditorCameraController();

	void UpdateProjection(uint32_t width, uint32_t height);
	void OnEvent(Volt::Event& e);

	void SetIsControllable(bool hovered);
	void ForceLooseControl();

	void SetTranslationSpeed(float speed) { m_translationSpeed = speed; }
	float GetTranslationSpeed() { return m_translationSpeed; }

	inline const bool HasControl() const { return m_isControllable; }
	void Focus(const glm::vec3& focusPoint);

	inline Ref<Volt::Camera> GetCamera() { return m_camera; }

private:
	enum class Mode
	{
		Fly,
		ArcBall
	};

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

	bool m_isControllable = false;
	bool m_isViewportHovered = false;

	inline static bool s_mouseEnabled = true;

	Mode m_cameraMode = Mode::Fly;
};
