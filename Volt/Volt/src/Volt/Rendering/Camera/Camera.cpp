#include "vtpch.h"
#include "Camera.h"

namespace Volt
{
	Camera::Camera(float fov, float aspect, float nearPlane, float farPlane)
		: myFieldOfView(fov), myAspecRatio(aspect), myNearPlane(nearPlane), myFarPlane(farPlane)
	{
		myProjectionMatrix = gem::perspective(gem::radians(myFieldOfView), aspect, myNearPlane, myFarPlane);
		myViewMatrix = gem::mat4(1.f);
		myIsOrthographic = false;

		RecalculateFrustum();
	}

	Camera::Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		: myNearPlane(nearPlane), myFarPlane(farPlane), myLeft(left), myRight(right), myBottom(bottom), myTop(top)
	{
		myProjectionMatrix = gem::ortho(left, right, bottom, top, -1.f, 1.f);
		myViewMatrix = gem::mat4(1.f);
		myIsOrthographic = true;

		RecalculateFrustum();
	}

	void Camera::SetPerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane)
	{
		myFieldOfView = fov;
		myAspecRatio = aspect;
		myNearPlane = nearPlane;
		myFarPlane = farPlane;

		myProjectionMatrix = gem::perspective(gem::radians(myFieldOfView), myAspecRatio, myNearPlane, myFarPlane);
		myIsOrthographic = false;

		RecalculateFrustum();
	}

	gem::vec3 Camera::ScreenToWorldCoords(const gem::vec2& someCoords, const gem::vec2& aSize)
	{
		float x = (someCoords.x / aSize.x) * 2.f - 1.f;
		float y = (someCoords.y / aSize.y) * 2.f - 1.f;

		gem::mat4 matInv = gem::inverse(myProjectionMatrix * myViewMatrix);

		gem::vec4 rayOrigin = matInv * gem::vec4(x, -y, 0, 1);
		gem::vec4 rayEnd = matInv * gem::vec4(x, -y, 1, 1);

		if (rayOrigin.w == 0 || rayEnd.w == 0)
		{
			return { 0,0,0 };
		}

		rayOrigin /= rayOrigin.w;
		rayEnd /= rayEnd.w;

		gem::vec3 rayDir = gem::normalize(rayEnd - rayOrigin);

		return rayDir;
	}

	void Camera::RecalculateFrustum()
	{
		const gem::vec3 forward = gem::normalize(GetForward());
		const gem::vec3 up = gem::normalize(GetUp());
		const gem::vec3 right = gem::normalize(GetRight());

		const gem::vec3 frontMulFar = myFarPlane * forward;

		if (!myIsOrthographic)
		{
			const float halfVSide = myFarPlane * std::tanf(gem::radians(myFieldOfView) * 0.5f);
			const float halfHSide = halfVSide * myAspecRatio;

			myFrustum.nearPlane = { myPosition + myNearPlane * forward, forward };
			myFrustum.farPlane = { myPosition + frontMulFar, -1.f * forward };

			myFrustum.rightPlane = { myPosition, gem::cross(up, frontMulFar - right * halfHSide) };
			myFrustum.leftPlane = { myPosition, gem::cross(frontMulFar + right * halfHSide, up) };

			myFrustum.topPlane = { myPosition, gem::cross(right, frontMulFar + up * halfVSide) };
			myFrustum.bottomPlane = { myPosition, gem::cross(frontMulFar - up * halfVSide, right) };
		}
		else
		{
			myFrustum.nearPlane = { myPosition + myNearPlane * forward, forward };
			myFrustum.farPlane = { myPosition + frontMulFar, -1.f * forward };
		
			myFrustum.rightPlane = { myPosition + gem::vec3{ myRight, 0.f, 0.f }, -1.f * right };
			myFrustum.leftPlane = { myPosition + gem::vec3{ myLeft, 0.f, 0.f }, right };
		
			myFrustum.topPlane = { myPosition + gem::vec3{ 0.f, myTop, 0.f }, up };
			myFrustum.bottomPlane = { myPosition + gem::vec3{ 0.f, myBottom, 0.f }, -1.f * up };
		}
	}

	void Camera::SetOrthographicProjection(float left, float right, float bottom, float top)
	{
		myProjectionMatrix = gem::ortho(left, right, bottom, top, -1.f, 1.f);
		myIsOrthographic = true;
	
		myLeft = left;
		myRight = right;
		myTop = top;
		myBottom = bottom;
	}

	gem::vec3 Camera::GetUp() const
	{
		return gem::rotate(GetOrientation(), gem::vec3{ 0.f, 1.f, 0.f });
	}

	gem::vec3 Camera::GetRight() const
	{
		return gem::rotate(GetOrientation(), gem::vec3{ 1.f, 0.f, 0.f });
	}

	gem::vec3 Camera::GetForward() const
	{
		return gem::rotate(GetOrientation(), gem::vec3{ 0.f, 0.f, 1.f });
	}

	gem::quat Camera::GetOrientation() const
	{
		return gem::quat(myRotation);
	}

	void Camera::RecalculateViewMatrix()
	{
		const float yawSign = GetUp().y < 0 ? -1.0f : 1.0f;

		const gem::vec3 lookAt = myPosition + GetForward();
		myViewMatrix = gem::lookAt(myPosition, lookAt, gem::vec3(0.f, yawSign, 0.f));

		RecalculateFrustum();
	}
}