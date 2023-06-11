#include "vtpch.h"
#include "PhysXUtilities.h"

#include "Volt/Physics/PhysicsLayer.h"

#include "Volt/Math/Math.h"

namespace Volt
{
	namespace PhysXUtilities
	{
		physx::PxTransform ToPhysXTransform(const glm::mat4& transform)
		{
			glm::vec3 trans, scale;
			glm::quat rot;
			Math::Decompose(transform, trans, rot, scale);

			physx::PxQuat r = ToPhysXQuat(rot);
			physx::PxVec3 p = ToPhysXVector(trans);

			return physx::PxTransform(p, r);
		}

		physx::PxTransform ToPhysXTransform(const glm::vec3& position, const glm::quat& rotation)
		{
			return physx::PxTransform(ToPhysXVector(position), ToPhysXQuat(rotation));
		}

		physx::PxMat44 ToPhysXMatrix(const glm::mat4& mat)
		{
			return *(physx::PxMat44*)&mat;
		}

		const physx::PxVec3& ToPhysXVector(const glm::vec3& vec)
		{
			return *(physx::PxVec3*)&vec;
		}

		const physx::PxVec4& ToPhysXVector(const glm::vec4& vec)
		{
			return *(physx::PxVec4*)&vec;
		}

		const physx::PxExtendedVec3 ToPhysXVectorExtended(const glm::vec3& vec)
		{
			return physx::PxExtendedVec3{ (double)vec.x, (double)vec.y, (double)vec.z };
		}

		physx::PxQuat ToPhysXQuat(const glm::quat& quat)
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

		glm::mat4 FromPhysXTransform(const physx::PxTransform& transform)
		{
			glm::quat rotation = FromPhysXQuat(transform.q);
			glm::vec3 position = FromPhysXVector(transform.p);
			return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation);
		}

		glm::mat4 FromPhysXMatrix(const physx::PxMat44& matrix)
		{
			return *(glm::mat4*)&matrix;
		}

		glm::vec3 FromPhysXVector(const physx::PxVec3& vector)
		{
			return *(glm::vec3*)&vector;
		}

		glm::vec3 FromPhysXVector(const physx::PxExtendedVec3& vector)
		{
			return { (float)vector.x, (float)vector.y, (float)vector.z };
		}

		glm::vec4 FromPhysXVector(const physx::PxVec4& vector)
		{
			return *(glm::vec4*)&vector;
		}

		glm::quat FromPhysXQuat(const physx::PxQuat& quat)
		{
			return { quat.w, quat.x, quat.y, quat.z };
		}

		CookingResult FromPhysXCookingResult(physx::PxConvexMeshCookingResult::Enum cookingResult)
		{
			switch (cookingResult)
			{
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
				case physx::PxTriangleMeshCookingResult::eLARGE_TRIANGLE: return CookingResult::LargeTriangle;
				case physx::PxTriangleMeshCookingResult::eFAILURE: return CookingResult::Failure;
			}

			return CookingResult::Failure;
		}
	}
}
