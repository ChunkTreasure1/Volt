#include "vtpch.h"
#include "Camera.h"
#include "gem/matrix/matrix_clip_space.h"

namespace Volt
{
	Camera::Camera(float fov, float aspect, float nearPlane, float farPlane, bool reverse)
		: myFieldOfView(fov), myAspecRatio(aspect), myNearPlane(nearPlane), myFarPlane(farPlane), myReversed(reverse)
	{
		if (reverse)
		{
			myProjectionMatrix = gem::perspective(gem::radians(myFieldOfView), aspect, myFarPlane, myNearPlane);
		}
		else
		{
			myProjectionMatrix = gem::perspective(gem::radians(myFieldOfView), aspect, myNearPlane, myFarPlane);
		}
		myViewMatrix = gem::mat4(1.f);
		myIsOrthographic = false;

		RecalculateFrustum();
	}

	Camera::Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		: myNearPlane(nearPlane), myFarPlane(farPlane), myLeft(left), myRight(right), myBottom(bottom), myTop(top)
	{
		myProjectionMatrix = gem::ortho(left, right, bottom, top, nearPlane, farPlane);
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

		if (myReversed)
		{
			myProjectionMatrix = gem::perspective(gem::radians(myFieldOfView), myAspecRatio, myFarPlane, myNearPlane);
		}
		else
		{
			myProjectionMatrix = gem::perspective(gem::radians(myFieldOfView), aspect, myNearPlane, myFarPlane);
		}

		myIsOrthographic = false;
		RecalculateFrustum();
	}

	void Camera::SetSubpixelOffset(const gem::vec2& offset)
	{
		mySubpixelOffset = offset;

		if (offset == 0.f)
		{
			return;
		}

		if (myReversed)
		{
			myProjectionMatrix = gem::perspective(gem::radians(myFieldOfView), myAspecRatio, myFarPlane, myNearPlane);
		}
		else
		{
			myProjectionMatrix = gem::perspective(gem::radians(myFieldOfView), myAspecRatio, myNearPlane, myFarPlane);
		}

		if (mySubpixelOffset != 0.f)
		{
			myProjectionMatrix = gem::translate(gem::mat4{ 1.f }, { offset.x, offset.y, 0.f }) * myProjectionMatrix;
		}
	}

	const std::vector<gem::vec4> Camera::GetFrustumCorners()
	{
		const auto inv = gem::inverse(myProjectionMatrix * myViewMatrix);

		std::vector<gem::vec4> frustumCorners;

		for (uint32_t x = 0; x < 2; ++x)
		{
			for (uint32_t y = 0; y < 2; ++y)
			{
				for (uint32_t z = 0; z < 2; ++z)
				{
					gem::vec4 pt = inv * gem::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
					if (pt.w > 0.f)
					{
						pt /= pt.w;
					}
					frustumCorners.push_back(pt);
				}
			}
		}

		return frustumCorners;
	}

	gem::vec3 Camera::ScreenToWorldRay(const gem::vec2& someCoords, const gem::vec2& aSize)
	{
		float x = (someCoords.x / aSize.x) * 2.f - 1.f;
		float y = (someCoords.y / aSize.y) * 2.f - 1.f;

		gem::mat4 tempProj = gem::perspective(gem::radians(myFieldOfView), myAspecRatio, myNearPlane, myFarPlane);

		gem::mat4 matInv = gem::inverse(tempProj * myViewMatrix);

		gem::vec4 rayOrigin = matInv * gem::vec4(x, -y, 0.f, 1.f);
		gem::vec4 rayEnd = matInv * gem::vec4(x, -y, 1.f, 1.f);

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
		myProjectionMatrix = gem::ortho(left, right, bottom, top, myNearPlane, myFarPlane);
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
