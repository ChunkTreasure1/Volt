#pragma once

#include "Volt/Rendering/Camera/Frustum.h"

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

		inline void SetProjection(const glm::mat4& projection) { myProjectionMatrix = projection; }
		inline void SetView(const glm::mat4& view) { myViewMatrix = view; }

		inline void SetPosition(const glm::vec3& pos) { myPosition = pos; RecalculateViewMatrix(); }
		inline void SetRotation(const glm::vec3& rot) { myRotation = rot; RecalculateViewMatrix(); }

		inline void SetNearPlane(float nearPlane) { myNearPlane = nearPlane; }
		inline void SetFarPlane(float farPlane) { myFarPlane = farPlane; }

		inline const glm::mat4& GetProjection() const { return myProjectionMatrix; }
		inline const glm::mat4& GetView() const { return myViewMatrix; }

		inline const glm::vec3& GetPosition() const { return myPosition; }
		inline const glm::vec3& GetRotation() const { return myRotation; }

		inline const float GetFieldOfView() const { return myFieldOfView; }
		inline const float GetAspectRatio() const { return myAspecRatio; }

		inline const float GetNearPlane() const { return myNearPlane; }
		inline const float GetFarPlane() const { return myFarPlane; }

		inline const float GetAperture() const { return myAperture; }
		inline const float GetShutterSpeed() const { return myShutterSpeed; }
		inline const float GetISO() const { return myISO; }

		inline void SetAperture(float value) { myAperture = value; }
		inline void SetShutterSpeed(float value) { myShutterSpeed = value; }
		inline void SetISO(float value) { myISO = value; }

		void SetSubpixelOffset(const glm::vec2& offset);
		inline const glm::vec2& GetSubpixelOffset() const { return mySubpixelOffset; }

		inline const Frustum& GetFrustum() const { return myFrustum; }
		const std::vector<glm::vec4> GetFrustumCorners();

		glm::vec3 ScreenToWorldRay(const glm::vec2& someCoords, const glm::vec2& aSize);

		glm::vec3 GetUp() const;
		glm::vec3 GetRight() const;
		glm::vec3 GetForward() const;

		glm::quat GetOrientation() const;

	private:
		void RecalculateViewMatrix();
		void RecalculateFrustum();

		glm::vec3 myPosition = { 0.f, 0.f, 0.f };
		glm::vec3 myRotation = { 0.f, 0.f, 0.f };

		glm::mat4 myProjectionMatrix = glm::mat4(1.f);
		glm::mat4 myViewMatrix = glm::mat4(1.f);

		glm::vec2 mySubpixelOffset = 0.f;

		bool myReversed = true;

		float myNearPlane = 0.f;
		float myFarPlane = 0.f;

		// Perspective
		float myFieldOfView = 0.f;
		float myAspecRatio = 0.f;

		// Orthographic
		float myLeft = 0.f;
		float myRight = 0.f;
		float myTop = 0.f;
		float myBottom = 0.f;

		//Physical settings
		float myAperture = 16.f;
		float myShutterSpeed = 1.f / 100.f;
		float myISO = 100.f;

		bool myIsOrthographic = false;
		Frustum myFrustum;
	};
}
