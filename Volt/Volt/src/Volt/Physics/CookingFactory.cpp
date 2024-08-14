#include "vtpch.h"
#include "CookingFactory.h"

#include "Volt/Physics/PhysXInternal.h"
#include "Volt/Physics/PhysXUtilities.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"

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

		VT_ASSERT_MSG(myCookingData, "Could not initialize cooking SDK!");
	}

	void CookingFactory::Shutdown()
	{
		myCookingData = nullptr;
	}

	CookingResult CookingFactory::CookMesh(MeshColliderComponent& colliderComp, bool, Vector<MeshColliderData>& outData)
	{
		if (colliderComp.isConvex)
		{
			Ref<Mesh> srcMesh = AssetManager::GetAsset<Mesh>(colliderComp.colliderMesh);
			if (!srcMesh || !srcMesh->IsValid())
			{
				return CookingResult::Failure;
			}

			Vector<MeshColliderData> colliderData;

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
				i++;
			}

			// Cache the cooked mesh
			{
				MeshColliderCacheData cachedData;
				cachedData.colliderData = colliderData;

				const std::string colliderName = std::to_string(srcMesh->handle) + "Convex";
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
			if (!srcMesh || !srcMesh->IsValid())
			{
				return CookingResult::Failure;
			}

			Vector<MeshColliderData> colliderData;

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

				i++;
			}

			// Cache the cooked mesh
			{
				MeshColliderCacheData cachedData;
				cachedData.colliderData = colliderData;

				const std::string colliderName = std::to_string(srcMesh->handle) + "Triangle";
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

		const auto& vertexContainer = srcMesh->GetVertexContainer();
		const auto& indices = srcMesh->GetIndices();
		const auto& submesh = srcMesh->GetSubMeshes().at(subMeshIndex);

		CookingResult cookingResult = CookingResult::Failure;

		convexDesc.points.count = submesh.vertexCount;
		convexDesc.points.stride = sizeof(glm::vec3);
		convexDesc.points.data = &vertexContainer.positions[submesh.vertexStartOffset];
		convexDesc.indices.count = submesh.indexCount / 3;
		convexDesc.indices.data = &indices[submesh.indexStartOffset / 3];
		convexDesc.indices.stride = sizeof(uint32_t) * 3;

		if (submesh.vertexCount >= convexDesc.vertexLimit)
		{
			VT_LOG(Warning, "Attempting to cook mesh with more than {0} vertices! Switching to quantized cooking!", convexDesc.vertexLimit);
			convexDesc.flags |= physx::PxConvexFlag::eQUANTIZE_INPUT | physx::PxConvexFlag::eSHIFT_VERTICES;
			convexDesc.quantizedCount = (uint16_t)submesh.vertexCount;
		}

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxConvexMeshCookingResult::Enum result;

		if (!PxCookConvexMesh(myCookingData->cookingParams, convexDesc, buf, &result))
		{
			VT_LOG(Error, "Failed to cook mesh!");

			cookingResult = PhysXUtilities::FromPhysXCookingResult(result);
			return cookingResult;
		}

		outData.data.Allocate(buf.getSize());
		outData.data.Copy(buf.getData(), buf.getSize());
		outData.transform = submesh.transform;
		cookingResult = CookingResult::Success;

		return cookingResult;
	}

	CookingResult CookingFactory::CookTriangleMesh(Ref<Mesh> srcMesh, uint32_t subMeshIndex, MeshColliderData& outData)
	{
		physx::PxTriangleMeshDesc triangleDesc{};
		physx::PxCookingParams cookingParams{ myCookingData->cookingParams };

		const auto& vertexContainer = srcMesh->GetVertexContainer();
		const auto& indices = srcMesh->GetIndices();
		const auto& submesh = srcMesh->GetSubMeshes().at(subMeshIndex);

		CookingResult cookingResult = CookingResult::Failure;
		triangleDesc.points.stride = sizeof(glm::vec3);
		triangleDesc.points.count = submesh.vertexCount;
		triangleDesc.points.data = &vertexContainer.positions[submesh.vertexStartOffset];

		triangleDesc.triangles.stride = sizeof(uint32_t) * 3;
		triangleDesc.triangles.count = submesh.indexCount / 3;
		triangleDesc.triangles.data = &indices[submesh.indexStartOffset / 3];

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxTriangleMeshCookingResult::Enum result;
		if (!PxCookTriangleMesh(myCookingData->cookingParams, triangleDesc, buf, &result))
		{
			VT_LOG(Error, "Failed to cook mesh!");

			cookingResult = PhysXUtilities::FromPhysXCookingResult(result);
			return cookingResult;
		}

		outData.data.Allocate(buf.getSize());
		outData.data.Copy(buf.getData(), buf.getSize());
		outData.transform = submesh.transform;
		cookingResult = CookingResult::Success;

		return cookingResult;
	}

	void CookingFactory::GenerateDebugMesh(const MeshColliderComponent& colliderComp, const Vector<MeshColliderData>& meshData)
	{
		if (!colliderComp.isConvex)
		{
			Vector<Vertex> vertices;
			Vector<uint32_t> indices;
			Vector<SubMesh> subMeshes;

			for (const Volt::MeshColliderData & data : meshData)
			{
				physx::PxDefaultMemoryInputData input(data.data.As<physx::PxU8>(), static_cast<physx::PxU32>(data.data.GetSize()));
				physx::PxTriangleMesh* trimesh = PhysXInternal::GetPhysXSDK().createTriangleMesh(input);

				if (!trimesh)
				{
					continue;
				}

				const uint32_t nbVerts = trimesh->getNbVertices();
				const physx::PxVec3* triangleVertices = trimesh->getVertices();
				const uint32_t nbTriangles = trimesh->getNbTriangles();
				const physx::PxU16* tris = (const physx::PxU16*)trimesh->getTriangles();

				vertices.reserve(vertices.size() + nbVerts);
				indices.reserve(indices.size() + nbTriangles * 3);

				SubMesh& submesh = subMeshes.emplace_back();
				submesh.vertexStartOffset = static_cast<uint32_t>(vertices.size());
				submesh.vertexCount = nbVerts;
				submesh.indexStartOffset = static_cast<uint32_t>(indices.size());
				submesh.indexCount = nbTriangles * 3;
				submesh.materialIndex = 0;
				submesh.transform = data.transform;

				for (uint32_t v = 0; v < nbVerts; v++)
				{
					Vertex& v1 = vertices.emplace_back();
					v1.position = PhysXUtilities::FromPhysXVector(triangleVertices[v]);
				}

				for (uint32_t tri = 0; tri < nbTriangles; tri++)
				{
					indices.emplace_back(tris[3 * tri + 0]);
					indices.emplace_back(tris[3 * tri + 2]);
					indices.emplace_back(tris[3 * tri + 1]);
				}

				trimesh->release();
			}

			if (vertices.size() > 0)
			{
				Ref<Material> material = AssetManager::GetAsset<Material>("Engine/Meshes/Primitives/SM_Cube.vtmat");

				MaterialTable materialTable{};
				materialTable.SetMaterial(material->handle, 0);

				Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices, materialTable, subMeshes);
				MeshColliderCache::AddTriangleDebugMesh(colliderComp.colliderMesh, mesh);
			}
		}
		else
		{
			Vector<Vertex> vertices;
			Vector<uint32_t> indices;
			Vector<SubMesh> subMeshes;

			for (const Volt::MeshColliderData& data : meshData)
			{
				physx::PxDefaultMemoryInputData input(data.data.As<physx::PxU8>(), static_cast<physx::PxU32>(data.data.GetSize()));
				physx::PxConvexMesh* convexMesh = PhysXInternal::GetPhysXSDK().createConvexMesh(input);

				if (!convexMesh)
				{
					continue;
				}

				// Based On: https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/ThirdParty/PhysX3/NvCloth/samples/SampleBase/renderer/ConvexRenderMesh.cpp
				const uint32_t nbPolygons = convexMesh->getNbPolygons();
				const physx::PxVec3* convexVertices = convexMesh->getVertices();
				const physx::PxU8* convexIndices = convexMesh->getIndexBuffer();

				uint32_t nbVertices = 0;
				uint32_t nbFaces = 0;
				uint32_t vertCounter = 0;
				uint32_t indexCounter = 0;

				SubMesh& submesh = subMeshes.emplace_back();
				submesh.vertexStartOffset = static_cast<uint32_t>(vertices.size());
				submesh.indexStartOffset = static_cast<uint32_t>(indices.size());

				for (uint32_t i = 0; i < nbPolygons; i++)
				{
					physx::PxHullPolygon polygon;
					convexMesh->getPolygonData(i, polygon);
					nbVertices += polygon.mNbVerts;
					nbFaces += (polygon.mNbVerts - 2) * 3;

					uint32_t vI0 = vertCounter;
					for (uint32_t vI = 0; vI < polygon.mNbVerts; vI++)
					{
						Vertex& v = vertices.emplace_back();
						v.position = PhysXUtilities::FromPhysXVector(convexVertices[convexIndices[polygon.mIndexBase + vI]]);
						vertCounter++;
					}

					for (uint32_t vI = 1; vI < uint32_t(polygon.mNbVerts) - 1; vI++)
					{
						indices.emplace_back(vI0);
						indices.emplace_back(vI0 + vI);
						indices.emplace_back(vI0 + vI + 1);
						indexCounter++;
					}
				}

				submesh.vertexCount = vertCounter;
				submesh.indexCount = indexCounter * 3;
				submesh.materialIndex = 0;
				submesh.transform = data.transform;

				convexMesh->release();
			}

			if (vertices.size() > 0)
			{
				Ref<Material> material = AssetManager::GetAsset<Material>("Engine/Meshes/Primitives/SM_Cube.vtmat");

				MaterialTable materialTable{};
				materialTable.SetMaterial(material->handle, 0);

				Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices, materialTable, subMeshes);
				MeshColliderCache::AddConvexDebugMesh(colliderComp.colliderMesh, mesh);
			}
		}
	}
}
