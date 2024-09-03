#pragma once

#include <entt.hpp>

namespace Volt
{
	class Scene;
	class Event;

	class AudioSystem
	{
	public:
		AudioSystem() = default;
		void RuntimeStart(entt::registry& registry, Weak<Scene> scene);
		void RuntimeStop(entt::registry& registry, Weak<Scene> scene);
		void Update(entt::registry& registry, Weak<Scene> scene, const float& aDeltaTime);
		void OnEvent(entt::registry& registry, Volt::Event& e);

	private:

		//AudioSources
		void UpdateAudioSources(entt::registry& registry, Weak<Scene> scene, const float& aDeltaTime);

		//AudioListeners
		void UpdateAudioListeners(entt::registry& registry, Weak<Scene> scene, const float& aDeltaTime);

		void UpdateAudioOcclusion(entt::registry& registry, Weak<Scene> scene);
		glm::vec3 CalculatePoint(const glm::vec3& posA, const glm::vec3& posB, float soundWidth, bool isPositive);
		int CastRay(const glm::vec3& posA, const glm::vec3& posB, bool debug);

		float fixedUpdateTimer = 0.f;

	};
}
