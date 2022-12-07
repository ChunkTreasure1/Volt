#pragma once
#include "Volt/Scene/Entity.h"

#include <gem/gem.h>
#include <stack>
#include <optional>

namespace Volt
{
	struct TransformComponent;

	class NavMeshAgent
	{
	public:
		NavMeshAgent();
		~NavMeshAgent();

		void StartNavigation();
		void StopNavigation();
		void SetTarget(gem::vec3 aPosition);
		std::optional<gem::vec3> GetCurrentTarget() const;
		bool HasArrived() const;

		void SetSpeed(const float& aSpeed) { mySpeed = aSpeed; };
		float GetSpeed() const { return mySpeed; }

		std::optional<gem::vec3> GetTarget() { return myTarget; }

	private:
		void Update(float aTimestep, Entity aEntity);
		void MoveToTarget(float aTimestep, Entity aEntity);
		std::stack<gem::vec3> FindPath(const gem::vec3& start, const gem::vec3& target);

	private:
		friend class NavigationsSystem;

		float mySpeed = 100.f;

		gem::vec3 myAgentPosition = { 0.f, 0.f, 0.f };
		std::optional<gem::vec3> myTarget;
		std::stack<gem::vec3> myPath;
		std::vector<gem::vec3> myPath2;

		bool myActive = true;

	};
}