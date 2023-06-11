#include "vtpch.h"
#include "Camera.h"

namespace Volt
{
	Camera::Camera(float fov, float aspect, float nearPlane, float farPlane, bool reverse)
		: myFieldOfView(fov), myAspecRatio(aspect), myNearPlane(nearPlane), myFarPlane(farPlane), myReversed(reverse)
	{
		if (reverse)
		{
			myProjectionMatrix = glm::perspective(glm::radians(myFieldOfView), aspect, myFarPlane, myNearPlane);
		}
		else
		{
			myProjectionMatrix = glm::perspective(glm::radians(myFieldOfView), aspect, myNearPlane, myFarPlane);
		}
		myViewMatrix = glm::mat4(1.f);
		myIsOrthographic = false;

		RecalculateFrustum();
	}

	Camera::Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		: myNearPlane(nearPlane), myFarPlane(farPlane), myLeft(left), myRight(right), myBottom(bottom), myTop(top)
	{
		myProjectionMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
		myViewMatrix = glm::mat4(1.f);
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
			myProjectionMatrix = glm::perspective(glm::radians(myFieldOfView), myAspecRatio, myFarPlane, myNearPlane);
		}
		else
		{
			myProjectionMatrix = glm::perspective(glm::radians(myFieldOfView), aspect, myNearPlane, myFarPlane);
		}

		myIsOrthographic = false;
		RecalculateFrustum();
	}

	void Camera::SetSubpixelOffset(const glm::vec2& offset)
	{
		mySubpixelOffset = offset;

		if (offset == glm::vec2(0.f))
		{
			return;
		}

		if (myReversed)
		{
			myProjectionMatrix = glm::perspective(glm::radians(myFieldOfView), myAspecRatio, myFarPlane, myNearPlane);
		}
		else
		{
			myProjectionMatrix = glm::perspective(glm::radians(myFieldOfView), myAspecRatio, myNearPlane, myFarPlane);
		}

		if (mySubpixelOffset != glm::vec2(0.f))
		{
			myProjectionMatrix = glm::translate(glm::mat4{ 1.f }, { offset.x, offset.y, 0.f }) * myProjectionMatrix;
		}
	}

	const std::vector<glm::vec4> Camera::GetFrustumCorners()
	{
		const auto inv = glm::inverse(myProjectionMatrix * myViewMatrix);

		std::vector<glm::vec4> frustumCorners;

		for (uint32_t x = 0; x < 2; ++x)
		{
			for (uint32_t y = 0; y < 2; ++y)
			{
				for (uint32_t z = 0; z < 2; ++z)
				{
					glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
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

	glm::vec3 Camera::ScreenToWorldRay(const glm::vec2& someCoords, const glm::vec2& aSize)
	{
		float x = (someCoords.x / aSize.x) * 2.f - 1.f;
		float y = (someCoords.y / aSize.y) * 2.f - 1.f;

		glm::mat4 tempProj = glm::perspective(glm::radians(myFieldOfView), myAspecRatio, myNearPlane, myFarPlane);

		glm::mat4 matInv = glm::inverse(tempProj * myViewMatrix);

		glm::vec4 rayOrigin = matInv * glm::vec4(x, -y, 0.f, 1.f);
		glm::vec4 rayEnd = matInv * glm::vec4(x, -y, 1.f, 1.f);

		if (rayOrigin.w == 0 || rayEnd.w == 0)
		{
			return { 0,0,0 };
		}

		rayOrigin /= rayOrigin.w;
		rayEnd /= rayEnd.w;

		glm::vec3 rayDir = glm::normalize(rayEnd - rayOrigin);

		return rayDir;
	}

	void Camera::RecalculateFrustum()
	{
		const glm::vec3 forward = glm::normalize(GetForward());
		const glm::vec3 up = glm::normalize(GetUp());
		const glm::vec3 right = glm::normalize(GetRight());

		const glm::vec3 frontMulFar = myFarPlane * forward;

		if (!myIsOrthographic)
		{
			const float halfVSide = myFarPlane * std::tanf(glm::radians(myFieldOfView) * 0.5f);
			const float halfHSide = halfVSide * myAspecRatio;

			myFrustum.nearPlane = { myPosition + myNearPlane * forward, forward };
			myFrustum.farPlane = { myPosition + frontMulFar, -1.f * forward };

			myFrustum.rightPlane = { myPosition, glm::cross(up, frontMulFar - right * halfHSide) };
			myFrustum.leftPlane = { myPosition, glm::cross(frontMulFar + right * halfHSide, up) };

			myFrustum.topPlane = { myPosition, glm::cross(right, frontMulFar + up * halfVSide) };
			myFrustum.bottomPlane = { myPosition, glm::cross(frontMulFar - up * halfVSide, right) };
		}
		else
		{
			myFrustum.nearPlane = { myPosition + myNearPlane * forward, forward };
			myFrustum.farPlane = { myPosition + frontMulFar, -1.f * forward };

			myFrustum.rightPlane = { myPosition + glm::vec3{ myRight, 0.f, 0.f }, -1.f * right };
			myFrustum.leftPlane = { myPosition + glm::vec3{ myLeft, 0.f, 0.f }, right };

			myFrustum.topPlane = { myPosition + glm::vec3{ 0.f, myTop, 0.f }, up };
			myFrustum.bottomPlane = { myPosition + glm::vec3{ 0.f, myBottom, 0.f }, -1.f * up };
		}
	}

	void Camera::SetOrthographicProjection(float left, float right, float bottom, float top)
	{
		myProjectionMatrix = glm::ortho(left, right, bottom, top, myNearPlane, myFarPlane);
		myIsOrthographic = true;

		myLeft = left;
		myRight = right;
		myTop = top;
		myBottom = bottom;
	}

	glm::vec3 Camera::GetUp() const
	{
		return glm::rotate(GetOrientation(), glm::vec3{ 0.f, 1.f, 0.f });
	}

	glm::vec3 Camera::GetRight() const
	{
		return glm::rotate(GetOrientation(), glm::vec3{ 1.f, 0.f, 0.f });
	}

	glm::vec3 Camera::GetForward() const
	{
		return glm::rotate(GetOrientation(), glm::vec3{ 0.f, 0.f, 1.f });
	}

	glm::quat Camera::GetOrientation() const
	{
		return glm::quat(myRotation);
	}

	void Camera::RecalculateViewMatrix()
	{
		const float yawSign = GetUp().y < 0 ? -1.0f : 1.0f;

		const glm::vec3 lookAt = myPosition + GetForward();
		myViewMatrix = glm::lookAt(myPosition, lookAt, glm::vec3(0.f, yawSign, 0.f));

		RecalculateFrustum();
	}
}
