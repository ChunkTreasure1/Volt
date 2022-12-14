#include "vtpch.h"
#include "FbxImporter.h"

#include "Volt/Log/Log.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/Animation/Animation.h"

#include "Volt/Rendering/Shader/ShaderRegistry.h"

#include <gem/gem.h>

// Based on Daniel Borghammars TGA FBX Importer

namespace Volt
{
	namespace Utility
	{
		uint32_t GetFramesPerSecond(FbxTime::EMode aMode)
		{
			switch (aMode)
			{
				case FbxTime::eDefaultMode: return 0;
				case FbxTime::eFrames120: return 120;
				case FbxTime::eFrames100: return 100;
				case FbxTime::eFrames60: return 60;
				case FbxTime::eFrames50: return 50;
				case FbxTime::eFrames48: return 48;
				case FbxTime::eFrames30: return 30;
				case FbxTime::eFrames30Drop: return 30;
				case FbxTime::eNTSCDropFrame: return 30;
				case FbxTime::eNTSCFullFrame: return 30;
				case FbxTime::ePAL: return 24;
				case FbxTime::eFrames24: return 24;
				case FbxTime::eFrames1000: return 1000;
				case FbxTime::eFilmFullFrame: return 0;
				case FbxTime::eCustom: return 0;
				case FbxTime::eFrames96: return 96;
				case FbxTime::eFrames72: return 72;
				case FbxTime::eFrames59dot94: return 60;
				case FbxTime::eFrames119dot88: return 120;
			}

			return 0;
		}

		gem::mat4 FbxMToMatrix(const FbxAMatrix& aMatrix)
		{
			gem::mat4 result;
			for (uint32_t i = 0; i < 4; i++)
			{
				FbxVector4 column = aMatrix.GetColumn(i);
				result[i] = gem::vec4((float)column[0], (float)column[1], (float)column[2], (float)column[3]);
			}

			return result;
		}

		gem::vec3 FbxVec4ToVec3(const FbxVector4& aVector)
		{
			gem::vec3 result;

			result.x = static_cast<float>(aVector[0]);
			result.y = static_cast<float>(aVector[1]);
			result.z = static_cast<float>(aVector[2]);

			return result;
		}

		gem::quat FbxQuatToQuat(const fbxsdk::FbxQuaternion& aQuat)
		{
			gem::quat result;

			result.x = static_cast<float>(aQuat[0]);
			result.y = static_cast<float>(aQuat[1]);
			result.z = static_cast<float>(aQuat[2]);
			result.w = static_cast<float>(aQuat[3]);

			return result;
		}
	}

	Ref<Mesh> FbxImporter::ImportMeshImpl(const std::filesystem::path& path)
	{
		FbxManager* sdkManager = FbxManager::Create();
		FbxIOSettings* ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT);

		ioSettings->SetBoolProp(IMP_FBX_MATERIAL, true);
		ioSettings->SetBoolProp(IMP_FBX_TEXTURE, false);
		ioSettings->SetBoolProp(IMP_FBX_LINK, false);
		ioSettings->SetBoolProp(IMP_FBX_SHAPE, false);
		ioSettings->SetBoolProp(IMP_FBX_GOBO, false);
		ioSettings->SetBoolProp(IMP_FBX_ANIMATION, false);
		ioSettings->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

		sdkManager->SetIOSettings(ioSettings);

		fbxsdk::FbxImporter* importer = fbxsdk::FbxImporter::Create(sdkManager, "");

		bool importStatus = importer->Initialize(path.string().c_str(), -1, sdkManager->GetIOSettings());
		if (!importStatus)
		{
			VT_CORE_ERROR("Unable to import file {0}!", path.string().c_str());
			return nullptr;
		}

		FbxScene* fbxScene = FbxScene::Create(sdkManager, "Scene");

		importer->Import(fbxScene);

		FbxGeometryConverter geomConverter(sdkManager);

		if (!geomConverter.Triangulate(fbxScene, true, true))
		{
			geomConverter.Triangulate(fbxScene, true, false);
		}

		FbxNode* rootNode = fbxScene->GetRootNode();

		std::vector<FbxNode*> geomNodes;
		FetchGeometryNodes(rootNode, geomNodes);

		for (auto node : geomNodes)
		{
			FbxMesh* fbxMesh = node->GetMesh();
			if (fbxMesh->GetElementBinormalCount() == 0 || fbxMesh->GetElementTangentCount() == 0)
			{
				if (!fbxMesh->GenerateTangentsData(0, true, false))
				{
					VT_CORE_ERROR("Unable to generate tangents/binormals for mesh {0}!", fbxMesh->GetName());
				}
			}
		}

		fbxsdk::FbxAxisSystem axisSystem(fbxsdk::FbxAxisSystem::eDirectX);
		axisSystem.DeepConvertScene(fbxScene);

		Ref<Mesh> mesh = CreateRef<Mesh>();
		Ref<Skeleton> skeleton = CreateRef<Skeleton>();
		mesh->myMaterial = CreateRef<Material>();
		mesh->myMaterial->myName = path.stem().string() + "_mat";

		ProcessSkeletonHierarchy(rootNode, skeleton);

		std::unordered_multimap<uint32_t, std::pair<uint32_t, float>> controlPointWeights;

		for (auto node : geomNodes)
		{
			ProcessJoints(node, controlPointWeights, skeleton);
			ProcessMesh(node, skeleton, fbxScene, controlPointWeights, mesh);
		}

		mesh->Construct();

		importer->Destroy();

		return mesh;
	}

	Ref<Skeleton> FbxImporter::ImportSkeletonImpl(const std::filesystem::path& path)
	{
		FbxManager* sdkManager = FbxManager::Create();
		FbxIOSettings* ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT);

		ioSettings->SetBoolProp(IMP_FBX_MATERIAL, false);
		ioSettings->SetBoolProp(IMP_FBX_TEXTURE, false);
		ioSettings->SetBoolProp(IMP_FBX_LINK, false);
		ioSettings->SetBoolProp(IMP_FBX_SHAPE, false);
		ioSettings->SetBoolProp(IMP_FBX_GOBO, false);
		ioSettings->SetBoolProp(IMP_FBX_ANIMATION, false);
		ioSettings->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

		sdkManager->SetIOSettings(ioSettings);

		fbxsdk::FbxImporter* importer = fbxsdk::FbxImporter::Create(sdkManager, "");

		bool importStatus = importer->Initialize(path.string().c_str(), -1, sdkManager->GetIOSettings());
		if (!importStatus)
		{
			VT_CORE_ERROR("Unable to import file {0}!", path.string().c_str());
			return nullptr;
		}

		FbxScene* fbxScene = FbxScene::Create(sdkManager, "Scene");

		fbxsdk::FbxAxisSystem axisSystem(fbxsdk::FbxAxisSystem::eDirectX);

		importer->Import(fbxScene);
		axisSystem.DeepConvertScene(fbxScene);

		FbxNode* rootNode = fbxScene->GetRootNode();
		Ref<Skeleton> skeleton = CreateRef<Skeleton>();

		ProcessSkeletonHierarchy(rootNode, skeleton);

		if (skeleton->myJoints.empty())
		{
			return nullptr;
		}

		std::unordered_multimap<uint32_t, std::pair<uint32_t, float>> controlPointWeights;

		std::vector<FbxNode*> geomNodes;
		FetchGeometryNodes(rootNode, geomNodes);

		for (const auto& node : geomNodes)
		{
			ProcessJoints(node, controlPointWeights, skeleton);
		}

		skeleton->myName = rootNode->GetName();

		importer->Destroy();
		return skeleton;
	}

	Ref<Animation> FbxImporter::ImportAnimationImpl(const std::filesystem::path& path)
	{
		FbxManager* sdkManager = FbxManager::Create();
		FbxIOSettings* ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT);

		ioSettings->SetBoolProp(IMP_FBX_MATERIAL, false);
		ioSettings->SetBoolProp(IMP_FBX_TEXTURE, false);
		ioSettings->SetBoolProp(IMP_FBX_LINK, false);
		ioSettings->SetBoolProp(IMP_FBX_SHAPE, false);
		ioSettings->SetBoolProp(IMP_FBX_GOBO, false);
		ioSettings->SetBoolProp(IMP_FBX_ANIMATION, true);
		ioSettings->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

		sdkManager->SetIOSettings(ioSettings);

		fbxsdk::FbxImporter* importer = fbxsdk::FbxImporter::Create(sdkManager, "");

		bool importStatus = importer->Initialize(path.string().c_str(), -1, sdkManager->GetIOSettings());
		if (!importStatus)
		{
			VT_CORE_ERROR("Unable to import file {0}!", path.string().c_str());
			return nullptr;
		}

		FbxScene* fbxScene = FbxScene::Create(sdkManager, "Scene");

		fbxsdk::FbxAxisSystem axisSystem(fbxsdk::FbxAxisSystem::eDirectX);

		importer->Import(fbxScene);
		axisSystem.DeepConvertScene(fbxScene);

		FbxNode* rootNode = fbxScene->GetRootNode();
		Ref<Skeleton> skeleton = CreateRef<Skeleton>();
		Ref<Animation> animation = CreateRef<Animation>();

		ProcessSkeletonHierarchy(rootNode, skeleton);

		const auto timeMode = fbxScene->GetGlobalSettings().GetTimeMode();
		animation->myFramesPerSecond = Utility::GetFramesPerSecond(timeMode);

		if (skeleton->myJoints.empty())
		{
			return nullptr;
		}

		FbxNode* skeletonRoot = fbxScene->FindNodeByName(skeleton->myJoints[0].name.c_str());

		const FbxVector4 translation = skeletonRoot->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 rotation = skeletonRoot->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 scale = skeletonRoot->GetGeometricScaling(FbxNode::eSourcePivot);
		const FbxAMatrix rootTransform = FbxAMatrix(translation, rotation, scale);

		FbxAnimStack* animStack = fbxScene->GetSrcObject<FbxAnimStack>(0);
		if (!animStack)
		{
			return nullptr;
		}

		FbxString stackName = animStack->GetName();
		FbxTakeInfo* takeInfo = fbxScene->GetTakeInfo(stackName);
		if (takeInfo)
		{
			const FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			const FbxTime end = takeInfo->mLocalTimeSpan.GetStop();

			const auto startFrame = start.GetFrameCount(timeMode);
			const auto endFrame = end.GetFrameCount(timeMode);

			const FbxLongLong animationLength = endFrame - startFrame + 1;
			animation->myDuration = (float)animationLength / (float)animation->myFramesPerSecond;

			animation->myFrames.resize(animationLength);

			uint32_t localFrameCounter = 0;

			for (FbxLongLong t = startFrame; t <= endFrame; t++)
			{
				FbxTime time;
				time.SetFrame(t, timeMode);

				animation->myFrames[localFrameCounter].localTransforms.resize(skeleton->myJoints.size());
				animation->myFrames[localFrameCounter].localTRS.resize(skeleton->myJoints.size());

				for (uint32_t j = 0; j < (uint32_t)skeleton->myJoints.size(); j++)
				{
					FbxNode* jointNode = fbxScene->FindNodeByName(skeleton->myJoints[j].name.c_str());
					const FbxAMatrix localTransform = jointNode->EvaluateLocalTransform(time);

					animation->myFrames[localFrameCounter].localTRS[j].position = Utility::FbxVec4ToVec3(localTransform.GetT());
					animation->myFrames[localFrameCounter].localTRS[j].scale = Utility::FbxVec4ToVec3(localTransform.GetS());
					animation->myFrames[localFrameCounter].localTRS[j].rotation = Utility::FbxQuatToQuat(localTransform.GetQ());

					const gem::mat4 localMat = Utility::FbxMToMatrix(localTransform);
					animation->myFrames[localFrameCounter].localTransforms[j] = localMat;
				}

				localFrameCounter++;
			}
		}
		else
		{
			VT_CORE_ERROR("No animation found in file {0}!", path.string());
		}

		if (animation->myFrames.empty())
		{
			VT_CORE_ERROR("Unable to load animation in file {0}!", path.string());
		}

		importer->Destroy();
		return animation;
	}

	void FbxImporter::ProcessMesh(FbxNode* fbxNode, Ref<Skeleton> skeleton, FbxScene* aScene, std::unordered_multimap<uint32_t, std::pair<uint32_t, float>>& aControlPointWeights, Ref<Mesh> mesh)
	{
		const uint32_t meshMaterialCount = fbxNode->GetMaterialCount();
		const bool hasMaterials = fbxNode->GetMaterialCount() != 0;
		FbxSurfaceMaterial* currentSceneMaterial = nullptr;
		FbxMesh* fbxMesh = fbxNode->GetMesh();

		for (int32_t i = 0; i < fbxNode->GetMaterialCount() || i == 0; i++)
		{
			int32_t matIndex = 0;

			if (hasMaterials)
			{
				for (int32_t j = 0; j < aScene->GetMaterialCount(); j++)
				{
					FbxSurfaceMaterial* sceneMaterial = aScene->GetMaterial(j);
					FbxSurfaceMaterial* nodeMaterial = fbxNode->GetMaterial(i);

					if (sceneMaterial == nodeMaterial)
					{
						currentSceneMaterial = sceneMaterial;
						matIndex = j;
						if (mesh->myMaterial->mySubMaterials.find(j) == mesh->myMaterial->mySubMaterials.end())
						{
							mesh->myMaterial->mySubMaterials[j] = SubMaterial::Create(sceneMaterial->GetName(), j, ShaderRegistry::Get("Deferred"));
						}

						break;
					}
				}
			}

			fbxsdk::FbxLayerElementMaterial* fbxElementMaterial = fbxMesh->GetElementMaterial();
			fbxsdk::FbxLayerElement::EMappingMode mappingMode = fbxElementMaterial->GetMappingMode();

			const int32_t triangleCount = fbxMesh->GetPolygonCount();
			uint32_t vertexCount = 0;

			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			vertices.reserve(triangleCount * 3);
			indices.reserve(triangleCount);

			for (uint32_t p = 0; p < triangleCount; p++)
			{
				if (hasMaterials)
				{
					const int32_t indexAtP = fbxElementMaterial->GetIndexArray().GetAt(p);
					FbxSurfaceMaterial* polygonMaterial = fbxNode->GetMaterial(indexAtP);
					if (currentSceneMaterial != polygonMaterial)
					{
						continue;
					}
				}

				const int32_t polySize = fbxMesh->GetPolygonSize(p);
				VT_CORE_ASSERT(polySize == 3, "Mesh must be fully triangulated!");

				for (int32_t v = 0; v < polySize; v++)
				{
					Vertex vertex;
					const int32_t ctrlPointIndex = fbxMesh->GetPolygonVertex(p, v);
					const int32_t polygonIndex = p * 3 + v;

					// Position
					{
						const FbxVector4 fbxVxPos = fbxMesh->GetControlPointAt(ctrlPointIndex);

						vertex.position.x = (float)fbxVxPos[0];
						vertex.position.y = (float)fbxVxPos[1];
						vertex.position.z = (float)fbxVxPos[2];
					}

					// UVs
					{
						const int32_t numUVs = fbxMesh->GetElementUVCount();
						const int32_t texUvIndex = fbxMesh->GetTextureUVIndex(p, v);

						for (int32_t uv = 0; uv < numUVs && uv < 4; uv++)
						{
							fbxsdk::FbxGeometryElementUV* uvElement = fbxMesh->GetElementUV(uv);
							const auto coord = uvElement->GetDirectArray().GetAt(texUvIndex);

							vertex.texCoords[uv].x = (float)coord.mData[0];
							vertex.texCoords[uv].y = 1.f - (float)coord.mData[1];
						}
					}

					// Normal
					{
						FbxVector4 normal;
						fbxsdk::FbxGeometryElementNormal* fbxElement = fbxMesh->GetElementNormal();
						if (fbxElement)
						{
							GetElementMappingData(fbxElement, ctrlPointIndex, polygonIndex, normal);

							vertex.normal.x = (float)normal[0];
							vertex.normal.y = (float)normal[1];
							vertex.normal.z = (float)normal[2];
						}
					}

					// Tangent
					{
						FbxVector4 tangent;
						fbxsdk::FbxGeometryElementTangent* fbxElement = fbxMesh->GetElementTangent();

						if (fbxElement)
						{
							GetElementMappingData(fbxElement, ctrlPointIndex, polygonIndex, tangent);

							vertex.tangent.x = (float)tangent[0];
							vertex.tangent.y = (float)tangent[1];
							vertex.tangent.z = (float)tangent[2];
						}
					}

					// Bitangent
					{
						FbxVector4 bitangent;
						fbxsdk::FbxGeometryElementBinormal* fbxElement = fbxMesh->GetElementBinormal();

						if (fbxElement)
						{
							GetElementMappingData(fbxElement, ctrlPointIndex, polygonIndex, bitangent);

							vertex.bitangent.x = (float)bitangent[0];
							vertex.bitangent.y = (float)bitangent[1];
							vertex.bitangent.z = (float)bitangent[2];
						}
					}

					if (!skeleton->myJoints.empty())
					{
						typedef std::unordered_multimap<uint32_t, std::pair<uint32_t, float>>::iterator mmIterator;

						std::pair<mmIterator, mmIterator> values = aControlPointWeights.equal_range(ctrlPointIndex);
						int32_t idx = 0;
						for (mmIterator it = values.first; it != values.second && idx < 4; ++it)
						{
							std::pair<uint32_t, float> boneAndWeight = it->second;

							switch (idx)
							{
								case 0:
									vertex.influences.x = boneAndWeight.first;
									vertex.weights.x = boneAndWeight.second;
									break;

								case 1:
									vertex.influences.y = boneAndWeight.first;
									vertex.weights.y = boneAndWeight.second;
									break;

								case 2:
									vertex.influences.z = boneAndWeight.first;
									vertex.weights.z = boneAndWeight.second;
									break;

								case 3:
									vertex.influences.w = boneAndWeight.first;
									vertex.weights.w = boneAndWeight.second;
									break;
							}
							idx++;
						}
					}

					fbxsdk::FbxColor fbxColors[4];
					const int32_t numColors = fbxMesh->GetElementVertexColorCount();
					for (int32_t col = 0; col < numColors && col < 4; col++)
					{
						fbxsdk::FbxGeometryElementVertexColor* colElement = fbxMesh->GetElementVertexColor(col);
						if (!colElement)
						{
							break;
						}

						switch (colElement->GetMappingMode())
						{
							case FbxGeometryElement::eByControlPoint:
							{
								switch (colElement->GetReferenceMode())
								{
									case FbxGeometryElement::eDirect:
									{
										fbxColors[col] = colElement->GetDirectArray().GetAt(ctrlPointIndex);
									}
									break;
									case FbxGeometryElement::eIndexToDirect:
									{
										const int Idx = colElement->GetIndexArray().GetAt(ctrlPointIndex);
										fbxColors[col] = colElement->GetDirectArray().GetAt(Idx);
									}
									break;
									default:
										throw std::exception("Invalid Reference Mode!");
								}
							}
							break;

							case FbxGeometryElement::eByPolygonVertex:
							{
								switch (colElement->GetReferenceMode())
								{
									case FbxGeometryElement::eDirect:
									{
										fbxColors[col] = colElement->GetDirectArray().GetAt(polygonIndex);
									}
									break;
									case FbxGeometryElement::eIndexToDirect:
									{
										const int Idx = colElement->GetIndexArray().GetAt(polygonIndex);
										fbxColors[col] = colElement->GetDirectArray().GetAt(Idx);
									}
									break;
									default:
										throw std::exception("Invalid Reference Mode!");
								}
							}
							break;
						}
					}

					for (uint32_t c = 0; c < 4; c++)
					{
						vertex.color[c].x = (float)fbxColors[c].mRed;
						vertex.color[c].y = (float)fbxColors[c].mGreen;
						vertex.color[c].z = (float)fbxColors[c].mBlue;
						vertex.color[c].w = (float)fbxColors[c].mAlpha;
					}

					size_t size = vertices.size();
					size_t currV = 0;

					for (currV = 0; currV < size; currV++)
					{
						if (vertex == vertices[currV])
						{
							break;
						}
					}

					if (currV == size)
					{
						vertices.emplace_back(vertex);
					}

					indices.emplace_back((uint32_t)currV);
					vertexCount++;
				}
			}

			// Copy new vertices into vector
			{
				size_t preVertexCount = mesh->myVertices.size();
				size_t preIndexCount = mesh->myIndices.size();

				mesh->myVertices.resize(preVertexCount + vertices.size());
				mesh->myIndices.resize(mesh->myIndices.size() + indices.size());

				memcpy_s(&mesh->myVertices[preVertexCount], sizeof(Vertex)* vertices.size(), vertices.data(), sizeof(Vertex)* vertices.size());
				memcpy_s(&mesh->myIndices[preIndexCount], sizeof(uint32_t)* indices.size(), indices.data(), sizeof(uint32_t)* indices.size());

				const auto fbxTranslation = fbxMesh->GetNode(0)->LclTranslation.Get();
				const auto fbxRotation = fbxMesh->GetNode(0)->LclRotation.Get();
				const auto fbxScale = fbxMesh->GetNode(0)->LclScaling.Get();

				const gem::vec3 position = { (float)fbxTranslation[0], (float)fbxTranslation[1], (float)fbxTranslation[2] };
				const gem::vec3 rotation = { (float)fbxRotation[0], (float)fbxRotation[1], (float)fbxRotation[2] };
				const gem::vec3 scale = { (float)fbxScale[0], (float)fbxScale[1], (float)fbxScale[2] };

				const gem::mat4 localTransform = gem::translate(gem::mat4(1.f), position) *
					gem::mat4_cast(gem::quat(rotation)) * gem::scale(gem::mat4(1.f), scale);

				auto& submesh = mesh->mySubMeshes.emplace_back();
				submesh.vertexCount = (uint32_t)vertices.size();
				submesh.indexCount = (uint32_t)indices.size();
				submesh.indexStartOffset = (uint32_t)preIndexCount;
				submesh.vertexStartOffset = (uint32_t)preVertexCount;
				submesh.materialIndex = matIndex;
				submesh.transform = localTransform;
				submesh.name = fbxMesh->GetName();
				submesh.GenerateHash();
			}
		}
	}

	void FbxImporter::FetchGeometryNodes(FbxNode* node, std::vector<FbxNode*>& outNodes)
	{
		if (node->GetNodeAttribute())
		{
			switch (node->GetNodeAttribute()->GetAttributeType())
			{
				case FbxNodeAttribute::eMesh:
				{
					if (node->GetMesh())
					{
						outNodes.emplace_back(node);
					}
				}
			}
		}

		for (uint32_t i = 0; i < (uint32_t)node->GetChildCount(); i++)
		{
			FetchGeometryNodes(node->GetChild(i), outNodes);
		}
	}

	void FbxImporter::GetElementMappingData(fbxsdk::FbxLayerElementTemplate<FbxVector4>* element, int32_t ctrlPointIndex, int32_t polyIndex, FbxVector4& outData)
	{
		switch (element->GetMappingMode())
		{
			case FbxGeometryElement::eByControlPoint:
			{
				switch (element->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{
						outData.mData[0] = element->GetDirectArray().GetAt(ctrlPointIndex).mData[0];
						outData.mData[1] = element->GetDirectArray().GetAt(ctrlPointIndex).mData[1];
						outData.mData[2] = element->GetDirectArray().GetAt(ctrlPointIndex).mData[2];
						outData.mData[3] = element->GetDirectArray().GetAt(ctrlPointIndex).mData[3];

						break;
					}

					case FbxGeometryElement::eIndexToDirect:
					{
						const int32_t index = element->GetIndexArray().GetAt(ctrlPointIndex);

						outData.mData[0] = element->GetDirectArray().GetAt(index).mData[0];
						outData.mData[1] = element->GetDirectArray().GetAt(index).mData[1];
						outData.mData[2] = element->GetDirectArray().GetAt(index).mData[2];
						outData.mData[3] = element->GetDirectArray().GetAt(index).mData[3];
						break;
					}

					default:
						VT_CORE_ASSERT(false, "Invalid reference mode!");
						break;
				}

				break;
			}

			case FbxGeometryElement::eByPolygonVertex:
			{
				switch (element->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{
						outData.mData[0] = element->GetDirectArray().GetAt(polyIndex).mData[0];
						outData.mData[1] = element->GetDirectArray().GetAt(polyIndex).mData[1];
						outData.mData[2] = element->GetDirectArray().GetAt(polyIndex).mData[2];
						outData.mData[3] = element->GetDirectArray().GetAt(polyIndex).mData[3];

						break;
					}

					case FbxGeometryElement::eIndexToDirect:
					{
						const int32_t index = element->GetIndexArray().GetAt(polyIndex);

						outData.mData[0] = element->GetDirectArray().GetAt(index).mData[0];
						outData.mData[1] = element->GetDirectArray().GetAt(index).mData[1];
						outData.mData[2] = element->GetDirectArray().GetAt(index).mData[2];
						outData.mData[3] = element->GetDirectArray().GetAt(index).mData[3];
					}

					default:
						VT_CORE_ASSERT(false, "Invalid reference mode!");
						break;
				}

				break;
			}

			default:
				VT_CORE_ASSERT(false, "Invalid mapping mode!");
				break;
		}
	}

	void FbxImporter::ProcessSkeletonHierarchy(FbxNode* aNode, Ref<Skeleton> aSkeleton)
	{
		for (int32_t i = 0; i < aNode->GetChildCount(); i++)
		{
			FbxNode* currNode = aNode->GetChild(i);
			ProcessSkeletonHierarchyRecursively(currNode, 0, -1, aSkeleton);
		}

		aSkeleton->myInverseBindPoses.resize(aSkeleton->myJoints.size());
	}

	void FbxImporter::ProcessSkeletonHierarchyRecursively(FbxNode* aNode, int32_t aIndex, int32_t aParent, Ref<Skeleton> aSkeleton)
	{
		bool added = false;

		if (aNode->GetNodeAttribute() && aNode->GetNodeAttribute()->GetAttributeType() && aNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			added = true;
			Skeleton::Joint currJoint;
			currJoint.parentIndex = aParent;
			currJoint.name = aNode->GetName();
			aSkeleton->myJoints.emplace_back(currJoint);
		}

		for (int i = 0; i < aNode->GetChildCount(); i++)
		{
			ProcessSkeletonHierarchyRecursively(aNode->GetChild(i), added ? (int32_t)aSkeleton->myJoints.size() : aIndex, added ? aIndex : aParent, aSkeleton);
		}
	}

	uint32_t FbxImporter::FindJointIndexByName(const std::string& aName, Ref<Skeleton> aSkeleton)
	{
		for (uint32_t i = 0; i < aSkeleton->myJoints.size(); i++)
		{
			if (aSkeleton->myJoints[i].name == aName)
			{
				return i;
			}
		}

		return (uint32_t)-1;
	}

	void FbxImporter::ProcessJoints(FbxNode* aNode, std::unordered_multimap<uint32_t, std::pair<uint32_t, float>>& aControlPointWeights, Ref<Skeleton> aSkeleton)
	{
		FbxMesh* currMesh = aNode->GetMesh();
		uint32_t numDeformers = currMesh->GetDeformerCount();

		const FbxVector4 lT = aNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = aNode->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = aNode->GetGeometricScaling(FbxNode::eSourcePivot);

		FbxAMatrix geomTransform = FbxAMatrix(lT, lR, lS);

		for (uint32_t deformerIndex = 0; deformerIndex < numDeformers; deformerIndex++)
		{
			FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
			if (!currSkin)
			{
				continue;
			}

			uint32_t numClusters = currSkin->GetClusterCount();
			for (uint32_t clusterIndex = 0; clusterIndex < numClusters; clusterIndex++)
			{
				FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
				std::string jointName = currCluster->GetLink()->GetName();
				uint32_t currentJoint = FindJointIndexByName(jointName, aSkeleton);

				FbxAMatrix transformMatrix;
				FbxAMatrix transformLinkMatrix;
				FbxAMatrix globalBindposeInverseMatrix;

				currCluster->GetTransformMatrix(transformMatrix);
				currCluster->GetTransformLinkMatrix(transformLinkMatrix);
				globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geomTransform;
				globalBindposeInverseMatrix = globalBindposeInverseMatrix.Transpose();

				aSkeleton->myInverseBindPoses[currentJoint] = Utility::FbxMToMatrix(globalBindposeInverseMatrix);

				uint32_t numIndices = currCluster->GetControlPointIndicesCount();
				for (uint32_t i = 0; i < numIndices; i++)
				{
					uint32_t controlPoint = currCluster->GetControlPointIndices()[i];
					float weight = static_cast<float>(currCluster->GetControlPointWeights()[i]);

					aControlPointWeights.insert({ controlPoint, { currentJoint, weight } });
				}
			}
		}
	}
}