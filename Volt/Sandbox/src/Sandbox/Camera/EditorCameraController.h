#pragma once

#include <Volt/Events/Event.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Events/MouseEvent.h>

#include <GEM/gem.h>

namespace Volt
{
	class Camera;
}

class EditorCameraController
{
public:
	EditorCameraController(float fov, float nearPlane, float farPlane);
	~EditorCameraController();

	void UpdateProjection(uint32_t width, uint32_t height);
	void OnEvent(Volt::Event& e);

	gem::vec3 GetScreenToRayDir();
	void SetViewportSize(gem::vec2 aSize) { m_ViewPortSize = aSize; };

	void SetIsControllable(bool hovered);
	void ForceLooseControl();

	inline const bool HasControl() const { return m_isControllable; }
	void Focus(const gem::vec3& focusPoint);

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

	void ArcBall(const gem::vec2& deltaPos);
	void ArcZoom(float deltaPos);

	const gem::vec3 CalculatePosition() const;

	Ref<Volt::Camera> m_camera;

	gem::vec2 m_ViewPortMousePos;
	gem::vec2 m_ViewPortSize;
	gem::vec2 m_lastMousePosition = { 0.f, 0.f };
	gem::vec3 m_position = { 0.f, 0.f, 0.f };
	gem::vec3 m_rotation = { 0.f, 0.f, 0.f };

	gem::vec3 m_positionDelta = { 0.f, 0.f, 0.f };
	float m_pitchDelta = 0.f;
	float m_yawDelta = 0.f;

	gem::vec3 m_focalPoint = { 0.f, 0.f, 0.f };
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

	Mode m_cameraMode = Mode::Fly;
};