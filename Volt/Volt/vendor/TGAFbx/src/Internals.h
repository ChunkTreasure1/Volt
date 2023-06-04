#pragma once
#include <algorithm>
#include "TgaFbxStructs.h"
#include "fbxsdk.h"

namespace TGA
{
	namespace FBX
	{
		class Internals
		{
			friend class Importer;

			struct VertexHash
			{
				size_t operator()(const Vertex& v) const;

			private:

				template<typename T>
				inline void hash_combine(size_t& s, const T& v) const
				{
					std::hash<T> h;
					s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
				}
			};

			struct VertexDuplicateData
			{
				unsigned int Idx;
				size_t Hash;
			};

			static std::string WideStringtoAnsi(const std::wstring& someString);
			static std::wstring AnsiStringToWide(const std::string& someString);

			static void FBXMatrixToArray(fbxsdk::FbxAMatrix anFbxMatrix, float anArray[16])
			{
				Matrix result;

				for (int i = 0; i < 4; i++)
				{
					// The FBX Matrix might need a Transpose!
					fbxsdk::FbxVector4 row = anFbxMatrix.GetRow(i);
					result(i + 1, 1) = static_cast<float>(row[0]);
					result(i + 1, 2) = static_cast<float>(row[1]);
					result(i + 1, 3) = static_cast<float>(row[2]);
					result(i + 1, 4) = static_cast<float>(row[3]);
				}

				memcpy(&anArray[0], result.Data, sizeof(float) * 16);
			}


			static std::array<float, 3> FbxVec4ToVec3(const FbxVector4& aVector)
			{
				std::array<float, 3> result;

				result[0] = static_cast<float>(aVector[0]);
				result[1] = static_cast<float>(aVector[1]);
				result[2] = static_cast<float>(aVector[2]);

				return result;
			}

			static std::array<float, 4> FbxQuatToQuat(const fbxsdk::FbxQuaternion& aQuat)
			{
				std::array<float, 4> result;

				result[0] = static_cast<float>(aQuat[0]);
				result[1] = static_cast<float>(aQuat[1]);
				result[2] = static_cast<float>(aQuat[2]);
				result[3] = static_cast<float>(aQuat[3]);

				return result;
			}


			static bool StringEndsWith(std::string const& value, std::string const& ending)
			{
				if (ending.size() > value.size()) return false;
				return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
			}

			static size_t GatherMeshNodes(FbxNode* aRootNode, std::vector<FbxNode*>& outNodes);
			static size_t GatherLODGroups(FbxNode* aRootNode, std::vector<FbxNode*>& outNodes);

			static bool GatherSkeleton(FbxNode* aRootNode, Skeleton& inOutSkeleton, std::vector<Skeleton::Bone>& outEventBones, std::vector<Skeleton::Socket>&
			                           outSockets, int someParentIndex = -1);

			static bool FbxMeshToMesh(FbxScene* aScene, FbxNode* aMeshNode, Skeleton& aSkeleton, std::vector<Mesh::Element>& outMeshes, bool bMergeDuplicateVertices);

			static void GetMaterialData(const FbxSurfaceMaterial* aMaterial, Material& outMaterial);

			static Texture GetMaterialChannelData(const FbxSurfaceMaterial* aMaterial, const char* aChannel);

			static void GetElementMappingData(FbxLayerElementTemplate<FbxVector4>* anElement, int aFbxContolPointIdx, int aPolygonIdx, FbxVector4& outData);
		};
	}
}
