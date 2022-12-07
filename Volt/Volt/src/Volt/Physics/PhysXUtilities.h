#pragma once

#include "Volt/Physics/PhysicsEnums.h"

#include <GEM/gem.h>
#include <PhysX/PxPhysicsAPI.h>

namespace Volt
{
	namespace PhysXUtilities
	{
		physx::PxTransform ToPhysXTransform(const gem::mat4& transform);
		physx::PxTransform ToPhysXTransform(const gem::vec3& position, const gem::vec3& rotation);
		physx::PxMat44 ToPhysXMatrix(const gem::mat4& mat);

		const physx::PxVec3& ToPhysXVector(const gem::vec3& vec);
		const physx::PxVec4& ToPhysXVector(const gem::vec4& vec);

		physx::PxQuat ToPhysXQuat(const gem::quat& quat);
		physx::PxFilterData CreateFilterDataFromLayer(uint32_t layerId, CollisionDetectionType collisionDetection);

		gem::mat4 FromPhysXTransform(const physx::PxTransform& transform);
		gem::mat4 FromPhysXMatrix(const physx::PxMat44& matrix);
		gem::vec3 FromPhysXVector(const physx::PxVec3& vector);
		gem::vec4 FromPhysXVector(const physx::PxVec4& vector);
		gem::quat FromPhysXQuat(const physx::PxQuat& quat);


		CookingResult FromPhysXCookingResult(physx::PxConvexMeshCookingResult::Enum cookingResult);
		CookingResult FromPhysXCookingResult(physx::PxTriangleMeshCookingResult::Enum cookingResult);
	};
}