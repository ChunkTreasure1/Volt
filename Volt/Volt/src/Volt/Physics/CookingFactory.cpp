#include "vtpch.h"
#include "CookingFactory.h"

#include "Volt/Physics/PhysXInternal.h"
#include "Volt/Physics/PhysXUtilities.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Physics/MeshColliderCache.h"

namespace Volt
{
	void CookingFactory::Initialize()
	{
		myCookingData = CreateScope<Data>(PhysXInternal::GetPhysXSDK().getTolerancesScale());
		myCookingData->cookingParams.meshWeldTolerance = 0.1f;
		myCookingData->cookingParams.meshPreprocessParams = physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
		myCookingData->cookingParams.midphaseDesc = physx::PxMeshMidPhase::eBVH34;

		VT_CORE_ASSERT(myCookingData, "Could not initialize cooking SDK!");
	}

	void CookingFactory::Shutdown()
	{
		myCookingData = nullptr;
	}

	CookingResult CookingFactory::CookMesh(MeshColliderComponent& colliderComp, bool, std::vector<MeshColliderData>& outData)
	{
		if (colliderComp.isConvex)
		{
			Ref<Mesh> srcMesh = AssetManager::GetAsset<Mesh>(colliderComp.colliderMesh);
			std::vector<MeshColliderData> colliderData;

			// Cook the mesh
			for (uint32_t i = 0; const auto & subMesh : srcMesh->GetSubMeshes())
			{
				subMesh;
				CookingResult result = CookConvexMesh(srcMesh, i, colliderData.emplace_back());
				if (result != CookingResult::Success)
				{
					for (auto& data : outData)
					{
						data.data.Release();
					}

					return result;
				}
			}

			// Cache the cooked mesh
			{
				MeshColliderCacheData cachedData;
				cachedData.colliderData = colliderData;

				const std::string colliderName = srcMesh->path.stem().string() + std::to_string(srcMesh->handle) + "Convex";
				MeshColliderCache::SaveToFile(colliderName, cachedData);
			}

			if (colliderComp.subMeshIndex != -1)
			{
				outData.emplace_back(colliderData.at(colliderComp.subMeshIndex));
			}
			else
			{
				outData = colliderData;
			}

			return CookingResult::Success;
		}
		else
		{
			Ref<Mesh> srcMesh = AssetManager::GetAsset<Mesh>(colliderComp.colliderMesh);
			std::vector<MeshColliderData> colliderData;  

			// Cook the mesh
			for (uint32_t i = 0; const auto & subMesh : srcMesh->GetSubMeshes())
			{
				subMesh;
				CookingResult result = CookTriangleMesh(srcMesh, i, colliderData.emplace_back());
				if (result != CookingResult::Success)
				{
					for (auto& data : outData)
					{
						data.data.Release();
					}

					return result;
				}
			}

			// Cache the cooked mesh
			{
				MeshColliderCacheData cachedData;
				cachedData.colliderData = colliderData;

				const std::string colliderName = srcMesh->path.stem().string() + std::to_string(srcMesh->handle) + "Triangle";
				MeshColliderCache::SaveToFile(colliderName, cachedData);
			}

			if (colliderComp.subMeshIndex != -1)
			{
				outData.emplace_back(colliderData.at(colliderComp.subMeshIndex));
			}
			else
			{
				outData = colliderData;
			}

			return CookingResult::Success;
		}
	}

	CookingResult CookingFactory::CookConvexMesh(Ref<Mesh> srcMesh, uint32_t subMeshIndex, MeshColliderData& outData)
	{
		physx::PxConvexMeshDesc convexDesc{};
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		physx::PxCookingParams cookingParams{ myCookingData->cookingParams };

		const auto& vertices = srcMesh->GetVertices();
		const auto& indices = srcMesh->GetIndices();
		const auto& submesh = srcMesh->GetSubMeshes().at(subMeshIndex);

		CookingResult cookingResult = CookingResult::Failure;

		convexDesc.points.count = submesh.vertexCount;
		convexDesc.points.stride = sizeof(Vertex);
		convexDesc.points.data = &vertices[submesh.vertexStartOffset];
		convexDesc.indices.count = submesh.indexCount / 3;
		convexDesc.indices.data = &indices[submesh.indexStartOffset / 3];
		convexDesc.indices.stride = sizeof(uint32_t) * 3;

		if (vertices.size() >= convexDesc.vertexLimit)
		{
			VT_CORE_WARN("Attempting to cook mesh with more than {0} vertices! Switching to quantized cooking!", convexDesc.vertexLimit);
			convexDesc.flags |= physx::PxConvexFlag::eQUANTIZE_INPUT | physx::PxConvexFlag::eSHIFT_VERTICES;
			convexDesc.quantizedCount = (uint16_t)submesh.vertexCount;
		}

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxConvexMeshCookingResult::Enum result;

		if (!PxCookConvexMesh(myCookingData->cookingParams, convexDesc, buf, &result))
		{
			VT_CORE_ERROR("Failed to cook mesh!");

			cookingResult = PhysXUtilities::FromPhysXCookingResult(result);
			return cookingResult;
		}

		outData.data.Allocate(buf.getSize());
		outData.data.Copy(buf.getData(), buf.getSize());
		cookingResult = CookingResult::Success;

		return cookingResult;
	}

	CookingResult CookingFactory::CookTriangleMesh(Ref<Mesh> srcMesh, uint32_t subMeshIndex, MeshColliderData& outData)
	{
		physx::PxTriangleMeshDesc triangleDesc{};
		physx::PxCookingParams cookingParams{ myCookingData->cookingParams };

		const auto& vertices = srcMesh->GetVertices();
		const auto& indices = srcMesh->GetIndices();
		const auto& submesh = srcMesh->GetSubMeshes().at(subMeshIndex);

		CookingResult cookingResult = CookingResult::Failure;
		triangleDesc.points.stride = sizeof(Vertex);
		triangleDesc.points.count = submesh.vertexCount;
		triangleDesc.points.data = &vertices[submesh.vertexStartOffset];

		triangleDesc.triangles.stride = sizeof(uint32_t) * 3;
		triangleDesc.triangles.count = submesh.indexCount / 3;
		triangleDesc.triangles.data = &indices[submesh.indexStartOffset / 3];

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxTriangleMeshCookingResult::Enum result;
		if (!PxCookTriangleMesh(myCookingData->cookingParams, triangleDesc, buf, &result))
		{
			VT_CORE_ERROR("Failed to cook mesh!");

			cookingResult = PhysXUtilities::FromPhysXCookingResult(result);
			return cookingResult;
		}

		outData.data.Allocate(buf.getSize());
		outData.data.Copy(buf.getData(), buf.getSize());
		cookingResult = CookingResult::Success;

		return cookingResult;
	}
}