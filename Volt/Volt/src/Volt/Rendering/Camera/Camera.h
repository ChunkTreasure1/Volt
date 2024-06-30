#pragma once

#include "Volt/Rendering/Camera/Frustum.h"
#include "Volt/Math/AABB.h"

#include <glm/glm.hpp>

namespace Volt
{
	class Camera
	{
	public:
		Camera(float fov, float aspect, float nearPlane, float farPlane, bool reverse = true);
		Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane);
		Camera() = default;
		~Camera() = default;

		void SetPerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane);
		void SetOrthographicProjection(float left, float right, float bottom, float top);

		inline void SetProjection(const glm::mat4& projection) { m_projectionMatrix = projection; }
		inline void SetView(const glm::mat4& view) { m_viewMatrix = view; }

		inline void SetPosition(const glm::vec3& pos) { m_position = pos; RecalculateViewMatrix(); }
		inline void SetRotation(const glm::vec3& rot) { m_rotation = rot; RecalculateViewMatrix(); }

		inline void SetNearPlane(float nearPlane) { m_nearPlane = nearPlane; }
		inline void SetFarPlane(float farPlane) { m_farPlane = farPlane; }

		inline const glm::mat4& GetProjection() const { return m_projectionMatrix; }
		inline const glm::mat4& GetView() const { return m_viewMatrix; }

		const glm::mat4 GetNonJitteredProjection() const;

		inline const glm::vec3& GetPosition() const { return m_position; }
		inline const glm::vec3& GetRotation() const { return m_rotation; }

		inline const float GetFieldOfView() const { return m_fieldOfView; }
		inline const float GetAspectRatio() const { return m_aspecRatio; }

		inline const float GetNearPlane() const { return m_nearPlane; }
		inline const float GetFarPlane() const { return m_farPlane; }

		inline const float GetAperture() const { return m_aperture; }
		inline const float GetShutterSpeed() const { return m_shutterSpeed; }
		inline const float GetISO() const { return m_ISO; }

		inline void SetAperture(float value) { m_aperture = value; }
		inline void SetShutterSpeed(float value) { m_shutterSpeed = value; }
		inline void SetISO(float value) { m_ISO = value; }

		void SetSubpixelOffset(const glm::vec2& offset);
		inline const glm::vec2& GetSubpixelOffset() const { return m_subpixelOffset; }

		inline const Frustum& GetFrustum() const { return m_frustum; }
		const std::vector<glm::vec4> GetFrustumCorners() const;
		const AABB GetOrthographicFrustum() const;

		glm::vec3 ScreenToWorldRay(const glm::vec2& someCoords, const glm::vec2& aSize);

		glm::vec3 GetUp() const;
		glm::vec3 GetRight() const;
		glm::vec3 GetForward() const;

		glm::quat GetOrientation() const;

	private:
		void RecalculateViewMatrix();
		void RecalculateFrustum();

		glm::vec3 m_position = { 0.f, 0.f, 0.f };
		glm::vec3 m_rotation = { 0.f, 0.f, 0.f };

		glm::mat4 m_projectionMatrix = glm::mat4(1.f);
		glm::mat4 m_viewMatrix = glm::mat4(1.f);

		glm::vec2 m_subpixelOffset = 0.f;

		bool m_reversed = true;

		float m_nearPlane = 0.f;
		float m_farPlane = 0.f;

		// Perspective
		float m_fieldOfView = 0.f;
		float m_aspecRatio = 0.f;

		// Orthographic
		float m_left = 0.f;
		float m_right = 0.f;
		float m_top = 0.f;
		float m_bottom = 0.f;

		//Physical settings
		float m_aperture = 16.f;
		float m_shutterSpeed = 1.f / 100.f;
		float m_ISO = 100.f;

		bool m_isOrthographic = false;
		Frustum m_frustum;
	};
}
