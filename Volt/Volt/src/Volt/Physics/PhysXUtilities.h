#pragma once

#include "Volt/Physics/PhysicsEnums.h"

#include <glm/glm.hpp>
#include <PhysX/PxPhysicsAPI.h>

namespace Volt
{
	namespace PhysXUtilities
	{
		physx::PxTransform ToPhysXTransform(const glm::mat4& transform);
		physx::PxTransform ToPhysXTransform(const glm::vec3& position, const glm::quat& rotation);
		physx::PxMat44 ToPhysXMatrix(const glm::mat4& mat);

		const physx::PxVec3& ToPhysXVector(const glm::vec3& vec);
		const physx::PxVec4& ToPhysXVector(const glm::vec4& vec);
		const physx::PxExtendedVec3 ToPhysXVectorExtended(const glm::vec3& vec);

		physx::PxQuat ToPhysXQuat(const glm::quat& quat);
		physx::PxFilterData CreateFilterDataFromLayer(uint32_t layerId, CollisionDetectionType collisionDetection);

		glm::mat4 FromPhysXTransform(const physx::PxTransform& transform);
		glm::mat4 FromPhysXMatrix(const physx::PxMat44& matrix);
		glm::vec3 FromPhysXVector(const physx::PxVec3& vector);
		glm::vec3 FromPhysXVector(const physx::PxExtendedVec3& vector);
		glm::vec4 FromPhysXVector(const physx::PxVec4& vector);
		glm::quat FromPhysXQuat(const physx::PxQuat& quat);


		CookingResult FromPhysXCookingResult(physx::PxConvexMeshCookingResult::Enum cookingResult);
		CookingResult FromPhysXCookingResult(physx::PxTriangleMeshCookingResult::Enum cookingResult);
	};
}