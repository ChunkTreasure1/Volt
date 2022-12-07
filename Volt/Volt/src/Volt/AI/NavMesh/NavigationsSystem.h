#pragma once
#include "Volt/Core/Base.h"
#include "Volt/Scene/Entity.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "NavMeshData.h"

namespace Volt
{
	class Scene;
	class NavMesh2;

	class NavigationsSystem
	{
	public:
		NavigationsSystem(Ref<Scene>& aScene);
		~NavigationsSystem();

		void SetNavMesh(Ref<Mesh> aNavMesh);
		const Ref<Mesh> GetNavMesh();
		const Ref<NavMesh> GetNavMeshData() const;
		const Ref<NavMesh2> GetNavMesh2() const;
		void OnRuntimeUpdate(float aTimestep);
		void OnSceneLoad();
		std::vector<Volt::Entity> GetBridges();

		void Draw();

		static NavigationsSystem& Get() { return *myInstance; };

	private:
		Entity myNavMeshEntity;
		Ref<Scene>& myCurrentScene;
		Ref<NavMesh> myNavMeshData;
		Ref<NavMesh2> myNavMesh2;

		inline static NavigationsSystem* myInstance;
	};
}