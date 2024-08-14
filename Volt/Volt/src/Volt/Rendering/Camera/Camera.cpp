#include "vtpch.h"
#include "Camera.h"

#include "Volt/Math/Math.h"

namespace Volt
{
	Camera::Camera(float fov, float aspect, float nearPlane, float farPlane, bool reverse)
		: m_fieldOfView(fov), m_aspecRatio(aspect), m_nearPlane(nearPlane), m_farPlane(farPlane), m_reversed(reverse)
	{
		if (reverse)
		{
			m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), aspect, m_farPlane, m_nearPlane);
		}
		else
		{
			m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), aspect, m_nearPlane, m_farPlane);
		}
		m_viewMatrix = glm::mat4(1.f);
		m_isOrthographic = false;

		RecalculateFrustum();
	}

	Camera::Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		: m_nearPlane(nearPlane), m_farPlane(farPlane), m_left(left), m_right(right), m_bottom(bottom), m_top(top)
	{
		m_projectionMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
		m_viewMatrix = glm::mat4(1.f);
		m_isOrthographic = true;

		RecalculateFrustum();
	}

	void Camera::SetPerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane)
	{
		m_fieldOfView = fov;
		m_aspecRatio = aspect;
		m_nearPlane = nearPlane;
		m_farPlane = farPlane;

		if (m_reversed)
		{
			m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), m_aspecRatio, m_farPlane, m_nearPlane);
		}
		else
		{
			m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), aspect, m_nearPlane, m_farPlane);
		}

		m_isOrthographic = false;
		RecalculateFrustum();
	}

	void Camera::SetSubpixelOffset(const glm::vec2& offset)
	{
		m_subpixelOffset = offset;

		if (glm::all(glm::equal(offset, { 0.f })))
		{
			return;
		}

		if (m_reversed)
		{
			m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), m_aspecRatio, m_farPlane, m_nearPlane);
		}
		else
		{
			m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), m_aspecRatio, m_nearPlane, m_farPlane);
		}

		if (glm::all(glm::notEqual(m_subpixelOffset, { 0.f })))
		{
			m_projectionMatrix = glm::translate(glm::mat4{ 1.f }, { offset.x, offset.y, 0.f })* m_projectionMatrix;
		}
	}

	const Vector<glm::vec4> Camera::GetFrustumCorners() const
	{
		const auto inv = glm::inverse(m_projectionMatrix * m_viewMatrix);

		Vector<glm::vec4> frustumCorners;

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

	const glm::vec4 Camera::GetFrustumCullingInfo() const
	{
		auto projTranspose = glm::transpose(m_projectionMatrix);

		const glm::vec4 frustumX = Math::NormalizePlane(projTranspose[3] + projTranspose[0]);
		const glm::vec4 frustumY = Math::NormalizePlane(projTranspose[3] + projTranspose[1]);

		return { frustumX.x, frustumX.z, frustumY.y, frustumY.z };
	}

	const AABB Camera::GetOrthographicFrustum() const
	{
		glm::vec3 min = std::numeric_limits<float>::max();
		glm::vec3 max = std::numeric_limits<float>::lowest();

		const auto frustumCorners = GetFrustumCorners();

		for (const auto& c : frustumCorners)
		{
			min = glm::min(min, glm::vec3(c));
			max = glm::max(max, glm::vec3(c));
		}

		return AABB{ min, max };
	}

	glm::vec3 Camera::ScreenToWorldRay(const glm::vec2& someCoords, const glm::vec2& aSize)
	{
		float x = (someCoords.x / aSize.x) * 2.f - 1.f;
		float y = (someCoords.y / aSize.y) * 2.f - 1.f;

		glm::mat4 tempProj = glm::perspective(glm::radians(m_fieldOfView), m_aspecRatio, m_nearPlane, m_farPlane);

		glm::mat4 matInv = glm::inverse(tempProj * m_viewMatrix);

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

		const glm::vec3 frontMulFar = m_farPlane * forward;

		if (!m_isOrthographic)
		{
			const float halfVSide = m_farPlane * std::tanf(glm::radians(m_fieldOfView) * 0.5f);
			const float halfHSide = halfVSide * m_aspecRatio;

			m_frustum.nearPlane = { m_position + m_nearPlane * forward, forward };
			m_frustum.farPlane = { m_position + frontMulFar, -1.f * forward };

			m_frustum.rightPlane = { m_position, glm::cross(up, frontMulFar - right * halfHSide) };
			m_frustum.leftPlane = { m_position, glm::cross(frontMulFar + right * halfHSide, up) };

			m_frustum.topPlane = { m_position, glm::cross(right, frontMulFar + up * halfVSide) };
			m_frustum.bottomPlane = { m_position, glm::cross(frontMulFar - up * halfVSide, right) };
		}
		else
		{
			m_frustum.nearPlane = { m_position + m_nearPlane * forward, forward };
			m_frustum.farPlane = { m_position + frontMulFar, -1.f * forward };

			m_frustum.rightPlane = { m_position + glm::vec3{ m_right, 0.f, 0.f }, -1.f * right };
			m_frustum.leftPlane = { m_position + glm::vec3{ m_left, 0.f, 0.f }, right };

			m_frustum.topPlane = { m_position + glm::vec3{ 0.f, m_top, 0.f }, up };
			m_frustum.bottomPlane = { m_position + glm::vec3{ 0.f, m_bottom, 0.f }, -1.f * up };
		}
	}

	void Camera::SetOrthographicProjection(float left, float right, float bottom, float top)
	{
		m_projectionMatrix = glm::ortho(left, right, bottom, top, m_nearPlane, m_farPlane);
		m_isOrthographic = true;

		m_left = left;
		m_right = right;
		m_top = top;
		m_bottom = bottom;
	}

	const glm::mat4 Camera::GetNonJitteredProjection() const
	{
		if (m_reversed)
		{
			return glm::perspective(glm::radians(m_fieldOfView), m_aspecRatio, m_farPlane, m_nearPlane);
		}
		else
		{
			return glm::perspective(glm::radians(m_fieldOfView), m_aspecRatio, m_nearPlane, m_farPlane);
		}
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
		return glm::quat(m_rotation);
	}

	void Camera::RecalculateViewMatrix()
	{
		const float yawSign = GetUp().y < 0 ? -1.0f : 1.0f;

		const glm::vec3 lookAt = m_position + GetForward();
		m_viewMatrix = glm::lookAt(m_position, lookAt, glm::vec3(0.f, yawSign, 0.f));

		RecalculateFrustum();
	}
}
