#pragma once

#include <CoreUtilities/Buffer/Buffer.h>
#include "Volt/Components/PhysicsComponents.h"
#include "Volt/Physics/PhysicsEnums.h"

#include <glm/glm.hpp>
#include <PhysX/PxPhysicsAPI.h>

namespace Volt
{
	struct MeshColliderData
	{
		Buffer data{};
		glm::mat4 transform = { 1.f };
	};

	struct MeshColliderCacheData
	{
		std::vector<MeshColliderData> colliderData;
	};

	class Mesh;
	class CookingFactory
	{
	public:
		static void Initialize();
		static void Shutdown();

		static CookingResult CookMesh(MeshColliderComponent& colliderComp, bool invalidateOld, std::vector<MeshColliderData>& outData);
		static void GenerateDebugMesh(const MeshColliderComponent& colliderComp, const std::vector<MeshColliderData>& meshData);

	private:
		struct CookingSettings
		{
			int32_t subMeshIndex = -1;
		};

		struct Data
		{
			Data(const physx::PxTolerancesScale& scale)
				: cookingParams(scale)
			{
			}

			physx::PxCookingParams cookingParams;
		};

		static CookingResult CookConvexMesh(Ref<Mesh> mesh, uint32_t subMeshIndex, MeshColliderData& outData);
		static CookingResult CookTriangleMesh(Ref<Mesh> srcMesh, uint32_t subMeshIndex, MeshColliderData& outData);


		inline static Scope<Data> myCookingData;
	};
}
