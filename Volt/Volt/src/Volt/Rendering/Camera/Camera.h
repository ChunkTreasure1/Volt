#pragma once

#include "Volt/Rendering/Camera/Frustum.h"

#include <gem/gem.h>
#include <GEM/quaternion/quaternion.h>

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

		inline void SetProjection(const gem::mat4& projection) { myProjectionMatrix = projection; }
		inline void SetView(const gem::mat4& view) { myViewMatrix = view; }

		inline void SetPosition(const gem::vec3& pos) { myPosition = pos; RecalculateViewMatrix(); }
		inline void SetRotation(const gem::vec3& rot) { myRotation = rot; RecalculateViewMatrix(); }

		inline void SetNearPlane(float nearPlane) { myNearPlane = nearPlane; }
		inline void SetFarPlane(float farPlane) { myFarPlane = farPlane; }

		inline const gem::mat4& GetProjection() const { return myProjectionMatrix; }
		inline const gem::mat4& GetView() const { return myViewMatrix; }

		inline const gem::vec3& GetPosition() const { return myPosition; }
		inline const gem::vec3& GetRotation() const { return myRotation; }

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

		void SetSubpixelOffset(const gem::vec2& offset);
		inline const gem::vec2& GetSubpixelOffset() const { return mySubpixelOffset; }

		inline const Frustum& GetFrustum() const { return myFrustum; }
		const std::vector<gem::vec4> GetFrustumCorners();

		gem::vec3 ScreenToWorldRay(const gem::vec2& someCoords, const gem::vec2& aSize);

		gem::vec3 GetUp() const;
		gem::vec3 GetRight() const;
		gem::vec3 GetForward() const;

		gem::quat GetOrientation() const;

	private:
		void RecalculateViewMatrix();
		void RecalculateFrustum();

		gem::vec3 myPosition = { 0.f, 0.f, 0.f };
		gem::vec3 myRotation = { 0.f, 0.f, 0.f };

		gem::mat4 myProjectionMatrix = gem::mat4(1.f);
		gem::mat4 myViewMatrix = gem::mat4(1.f);

		gem::vec2 mySubpixelOffset = 0.f;

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
