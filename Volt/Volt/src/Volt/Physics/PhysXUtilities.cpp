#include "vtpch.h"
#include "PhysXUtilities.h"

#include "Volt/Physics/PhysicsLayer.h"

namespace Volt
{
	namespace PhysXUtilities
	{
		physx::PxTransform ToPhysXTransform(const gem::mat4& transform)
		{
			gem::vec3 trans, rot, scale;
			gem::decompose(transform, trans, rot, scale);

			physx::PxQuat r = ToPhysXQuat(gem::quat(rot));
			physx::PxVec3 p = ToPhysXVector(trans);

			return physx::PxTransform(p, r);
		}

		physx::PxTransform ToPhysXTransform(const gem::vec3& position, const gem::vec3& rotation)
		{
			return physx::PxTransform(ToPhysXVector(position), ToPhysXQuat(gem::quat(rotation)));
		}

		physx::PxMat44 ToPhysXMatrix(const gem::mat4& mat)
		{
			return *(physx::PxMat44*)&mat;
		}

		const physx::PxVec3& ToPhysXVector(const gem::vec3& vec)
		{
			return *(physx::PxVec3*)&vec;
		}

		const physx::PxVec4& ToPhysXVector(const gem::vec4& vec)
		{
			return *(physx::PxVec4*)&vec;
		}

		physx::PxQuat ToPhysXQuat(const gem::quat& quat)
		{
			return { quat.x, quat.y, quat.z, quat.w };
		}

		physx::PxFilterData CreateFilterDataFromLayer(uint32_t layerId, CollisionDetectionType collisionDetection)
		{
			const auto& layer = PhysicsLayerManager::GetLayer(layerId);

			physx::PxFilterData data{};
			data.word0 = layer.bitValue;
			data.word1 = layer.collidesWith;
			data.word2 = (uint32_t)collisionDetection;

			return data;
		}

		gem::mat4 FromPhysXTransform(const physx::PxTransform& transform)
		{
			gem::quat rotation = FromPhysXQuat(transform.q);
			gem::vec3 position = FromPhysXVector(transform.p);
			return gem::translate(gem::mat4(1.0f), position) * gem::mat4_cast(rotation);
		}

		gem::mat4 FromPhysXMatrix(const physx::PxMat44& matrix)
		{
			return *(gem::mat4*)&matrix;
		}

		gem::vec3 FromPhysXVector(const physx::PxVec3& vector)
		{
			return *(gem::vec3*)&vector;
		}

		gem::vec4 FromPhysXVector(const physx::PxVec4& vector)
		{
			return *(gem::vec4*)&vector;
		}

		gem::quat FromPhysXQuat(const physx::PxQuat& quat)
		{
			return { quat.w, quat.x, quat.y, quat.z };
		}

		CookingResult FromPhysXCookingResult(physx::PxConvexMeshCookingResult::Enum cookingResult)
		{
			switch (cookingResult)
			{
				case physx::PxConvexMeshCookingResult::eSUCCESS: return CookingResult::Success;
				case physx::PxConvexMeshCookingResult::eZERO_AREA_TEST_FAILED: return CookingResult::ZeroAreaTestFailed;
				case physx::PxConvexMeshCookingResult::ePOLYGONS_LIMIT_REACHED: return CookingResult::PolygonLimitReached;
				case physx::PxConvexMeshCookingResult::eFAILURE: return CookingResult::Failure;
			}

			return CookingResult::Failure;
		}

		CookingResult FromPhysXCookingResult(physx::PxTriangleMeshCookingResult::Enum cookingResult)
		{
			switch (cookingResult)
			{
				case physx::PxTriangleMeshCookingResult::eSUCCESS: return CookingResult::Success;
				case physx::PxTriangleMeshCookingResult::eLARGE_TRIANGLE: return CookingResult::LargeTriangle;
				case physx::PxTriangleMeshCookingResult::eFAILURE: return CookingResult::Failure;
			}

			return CookingResult::Failure;
		}
	}
}