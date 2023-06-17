#pragma once
namespace Wire
{
	class Registry;
}

namespace Volt
{
	class Scene;
	class Event;

	class AudioSystem
	{
	public:
		AudioSystem() = default;
		void RuntimeStart(Wire::Registry& registry, Scene* scene);
		void RuntimeStop(Wire::Registry& registry, Scene* scene);
		void Update(Wire::Registry& registry, Scene* scene, const float& aDeltaTime);
		void OnEvent(Wire::Registry& registry, Volt::Event& e);

	private:

		//AudioSources
		void UpdateAudioSources(Wire::Registry& registry, Scene* scene, const float& aDeltaTime);

		//AudioListeners
		void UpdateAudioListeners(Wire::Registry& registry, Scene* scene, const float& aDeltaTime);

		void UpdateAudioOcclusion(Wire::Registry& registry, Scene* scene);
		gem::vec3 CalculatePoint(const gem::vec3& posA, const gem::vec3& posB, float soundWidth, bool isPositive);
		int CastRay(const gem::vec3& posA, const gem::vec3& posB, bool debug);

		float fixedUpdateTimer = 0.f;

	};
}