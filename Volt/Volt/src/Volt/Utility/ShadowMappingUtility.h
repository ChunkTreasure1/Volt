#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Rendering/Camera/Camera.h"

#include <vector>
#include <GEM/gem.h>

namespace Volt
{
	namespace Utility
	{
		inline gem::mat4 CalculateLightSpaceMatrix(Ref<Camera> sceneCamera, const float nearPlane, const float farPlane, const gem::vec3& lightDirection)
		{
			Ref<Camera> frustumCamera = CreateRef<Camera>(sceneCamera->GetFieldOfView(), sceneCamera->GetAspectRatio(), nearPlane, farPlane, false);
			frustumCamera->SetView(sceneCamera->GetView());
			const auto corners = frustumCamera->GetFrustumCorners();

			gem::vec3 center = 0.f;
			for (const auto& c : corners)
			{
				center += gem::vec3{ c };
			}

			center /= (float)corners.size();

			const gem::mat4 view = gem::lookAtLH(lightDirection + center, center, { 0.f, 1.f, 0.f });

			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::lowest();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::lowest();
			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::lowest();
			for (const auto& v : corners)
			{
				const auto trf = view * v;
				minX = std::min(minX, trf.x);
				maxX = std::max(maxX, trf.x);
				minY = std::min(minY, trf.y);
				maxY = std::max(maxY, trf.y);
				minZ = std::min(minZ, trf.z);
				maxZ = std::max(maxZ, trf.z);
			}

			// Tune this parameter according to the scene
			constexpr float zMult = 10.0f;
			if (minZ < 0)
			{
				minZ *= zMult;
			}
			else
			{
				minZ /= zMult;
			}
			if (maxZ < 0)
			{
				maxZ /= zMult;
			}
			else
			{
				maxZ *= zMult;
			}

			const gem::mat4 lightProjection = gem::orthoLH(minX, maxX, minY, maxY, minZ, maxZ);

			return lightProjection * view;
		}

		inline const std::vector<gem::mat4> CalculateCascadeMatrices(Ref<Camera> sceneCamera, const gem::vec3& lightDirection, const std::vector<float>& cascades)
		{
			std::vector<gem::mat4> result{};
			for (size_t i = 0; i < cascades.size() + 1; i++)
			{
				if (i == 0)
				{
					result.emplace_back(CalculateLightSpaceMatrix(sceneCamera, sceneCamera->GetNearPlane(), cascades.at(i), lightDirection));
				}
				else if (i < cascades.size())
				{
					result.emplace_back(CalculateLightSpaceMatrix(sceneCamera, cascades.at(i - 1), cascades.at(i), lightDirection));
				}
				else
				{
					result.emplace_back(CalculateLightSpaceMatrix(sceneCamera, cascades.at(i - 1), sceneCamera->GetFarPlane(), lightDirection));
				}
			}

			return result;
		}
	}
}
