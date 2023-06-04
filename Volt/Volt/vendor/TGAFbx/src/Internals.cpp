#include "TGAFbx.pch.h"
#include "Internals.h"
#define NOMINMAX
#include "Windows.h"

namespace TGA
{
	namespace FBX
	{
		size_t Internals::VertexHash::operator()(const Vertex& v) const
		{
			size_t result = 0;
			hash_combine(result, v.Position[0]);
			hash_combine(result, v.Position[1]);
			hash_combine(result, v.Position[2]);
			hash_combine(result, v.VertexColors[0][0]);
			hash_combine(result, v.VertexColors[0][1]);
			hash_combine(result, v.VertexColors[0][2]);
			hash_combine(result, v.VertexColors[0][3]);
			hash_combine(result, v.VertexColors[1][0]);
			hash_combine(result, v.VertexColors[1][1]);
			hash_combine(result, v.VertexColors[1][2]);
			hash_combine(result, v.VertexColors[1][3]);
			hash_combine(result, v.VertexColors[2][0]);
			hash_combine(result, v.VertexColors[2][1]);
			hash_combine(result, v.VertexColors[2][2]);
			hash_combine(result, v.VertexColors[2][3]);
			hash_combine(result, v.VertexColors[3][0]);
			hash_combine(result, v.VertexColors[3][1]);
			hash_combine(result, v.VertexColors[3][2]);
			hash_combine(result, v.VertexColors[3][3]);
			hash_combine(result, v.UVs[0][0]);
			hash_combine(result, v.UVs[0][1]);
			hash_combine(result, v.UVs[1][0]);
			hash_combine(result, v.UVs[1][1]);
			hash_combine(result, v.UVs[2][0]);
			hash_combine(result, v.UVs[2][1]);
			hash_combine(result, v.UVs[3][0]);
			hash_combine(result, v.UVs[3][1]);
			hash_combine(result, v.Normal[0]);
			hash_combine(result, v.Normal[1]);
			hash_combine(result, v.Normal[2]);
			hash_combine(result, v.Tangent[0]);
			hash_combine(result, v.Tangent[1]);
			hash_combine(result, v.Tangent[2]);
			hash_combine(result, v.BiNormal[0]);
			hash_combine(result, v.BiNormal[1]);
			hash_combine(result, v.BiNormal[2]);
			// This MAY break things and requires testing.
			hash_combine(result, v.BoneIDs[0]);
			hash_combine(result, v.BoneIDs[1]);
			hash_combine(result, v.BoneIDs[2]);
			hash_combine(result, v.BoneIDs[3]);
			hash_combine(result, v.BoneWeights[0]);
			hash_combine(result, v.BoneWeights[1]);
			hash_combine(result, v.BoneWeights[2]);
			hash_combine(result, v.BoneWeights[3]);

			return result;
		}

		std::string Internals::WideStringtoAnsi(const std::wstring& someString)
		{
			const int sLength = static_cast<int>(someString.length());
			const int len = WideCharToMultiByte(CP_ACP, 0, someString.c_str(), sLength, 0, 0, 0, 0);
			std::string result(len, L'\0');
			WideCharToMultiByte(CP_ACP, 0, someString.c_str(), sLength, &result[0], len, 0, 0);
			return result;
		}

		std::wstring Internals::AnsiStringToWide(const std::string& someString)
		{
			const int sLength = static_cast<int>(someString.length());
			const int len = MultiByteToWideChar(CP_ACP, 0, someString.c_str(), sLength, 0, 0);
			std::wstring result(len, L'\0');
			MultiByteToWideChar(CP_ACP, 0, someString.c_str(), sLength, &result[0], len);
			return result;
		}

		size_t Internals::GatherMeshNodes(FbxNode* aRootNode, std::vector<FbxNode*>& outNodes)
		{
			// For each child of our current node...
			for (int i = 0; i < aRootNode->GetChildCount(); i++)
			{
				// Get the node attribute if it has one, the data that denotes what type this node is.
				// If there is no type it's usually just an organizational node (folder).
				FbxNode* childNode = aRootNode->GetChild(i);
				const FbxNodeAttribute* childAttribute = childNode->GetNodeAttribute();
				if (!childAttribute)
				{
					// Even if it's not a valid node, step down since it may contain meshes.
					GatherMeshNodes(childNode, outNodes);
				}
				else
				{
					const FbxNodeAttribute::EType childAttributeType = childAttribute->GetAttributeType();

					// Check if this is a mesh node.			        
					if (childAttributeType == FbxNodeAttribute::eMesh)
					{
						// If it is, gather the node.
						outNodes.push_back(childNode);
					}

					GatherMeshNodes(childNode, outNodes);
				}
			}

			// Good idea to return how many meshes we found. Might be useful later.
			return static_cast<unsigned int>(outNodes.size());
		}

		size_t Internals::GatherLODGroups(FbxNode* aRootNode, std::vector<FbxNode*>& outNodes)
		{
			for (int i = 0; i < aRootNode->GetChildCount(); i++)
			{
				FbxNode* childNode = aRootNode->GetChild(i);
				const FbxNodeAttribute* childAttribute = childNode->GetNodeAttribute();
				if (!childAttribute)
				{
					GatherLODGroups(childNode, outNodes);
				}
				else
				{
					const FbxNodeAttribute::EType childAttributeType = childAttribute->GetAttributeType();

					if (childAttributeType == FbxNodeAttribute::eLODGroup)
					{
						outNodes.push_back(childNode);
					}
					else // We only want to find root-level LOD groups. I.e. not those below another LODGroup
					{
						GatherLODGroups(childNode, outNodes);
					}					
				}
			}

			return static_cast<unsigned int>(outNodes.size());
		}

		bool Internals::GatherSkeleton(FbxNode* aRootNode, Skeleton& inOutSkeleton, std::vector<Skeleton::Bone>& outEventBones, std::vector<Skeleton::Socket>& outSockets, int someParentIndex)
		{
			int myIndex = someParentIndex;
#if _DEBUG
			if(FbxNodeAttribute* tempAttribute = aRootNode->GetNodeAttribute())
			{
				FbxNodeAttribute::EType tempAttributeType = tempAttribute->GetAttributeType();
				std::string nodeName = aRootNode->GetName();
				int a = 1;
			}
#endif

			if (aRootNode->GetNodeAttribute()
				/*aRootNode->GetNodeAttribute()->GetAttributeType() &&*/
				/*aRootNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton*/)
			{

				switch(aRootNode->GetNodeAttribute()->GetAttributeType())
				{
				case FbxNodeAttribute::eSkeleton:
					{
						// We're a bone, process as normal
						Skeleton::Bone bone;
						bone.NamespaceName = aRootNode->GetName();
						bone.ParentIdx = someParentIndex;

						bone.Name = bone.NamespaceName;
						if(const size_t pos = bone.Name.find_last_of(':'); pos != std::string::npos)
						{
							bone.Name = bone.Name.substr(pos + 1);
						}
						inOutSkeleton.Bones.push_back(bone);

						myIndex = static_cast<int>(inOutSkeleton.Bones.size() - 1ULL);

						if (inOutSkeleton.BoneNameToIndex.find(bone.Name) != inOutSkeleton.BoneNameToIndex.end())
						{
							std::throw_with_nested(std::runtime_error("Found more than one bone with the same name [" + bone.NamespaceName + "]!"));
						}

						inOutSkeleton.BoneNameToIndex.insert({ bone.Name, myIndex });

						if (someParentIndex >= 0)
						{
							inOutSkeleton.Bones[someParentIndex].Children.push_back(myIndex);
						}
					}
					break;
				case FbxNodeAttribute::eNull:
					{
						// We're a something or other. Might be an Event or Socket.
						
						const std::string nodeName = aRootNode->GetName();

						if (StringEndsWith(nodeName, "_evt") || StringEndsWith(nodeName, "_event"))
						{
							Skeleton::Bone bone;
							bone.NamespaceName = nodeName;
							bone.ParentIdx = someParentIndex;

							outEventBones.push_back(bone);
						}
						else if (StringEndsWith(nodeName, "_skt") || StringEndsWith(nodeName, "_socket"))
						{
							Skeleton::Socket socket;
							socket.Name = nodeName;
							socket.ParentBoneIdx = someParentIndex;

							FbxAMatrix globalFrameTransform;
							globalFrameTransform = aRootNode->EvaluateGlobalTransform(0);
							Matrix GlobalTransform;
							Internals::FBXMatrixToArray(globalFrameTransform, GlobalTransform.Data);
							outSockets.emplace_back(socket);
						}
					}
					break;
				default:
					{
						
					}
					break;
				}
			}

			for (int i = 0; i < aRootNode->GetChildCount(); i++)
			{
				GatherSkeleton(aRootNode->GetChild(i), inOutSkeleton, outEventBones, outSockets, myIndex);
			}

			return !inOutSkeleton.Bones.empty();
		}

		Texture Internals::GetMaterialChannelData(const FbxSurfaceMaterial* aMaterial, const char* aChannel)
		{
			Texture result;
			const FbxProperty channelProperty = aMaterial->FindProperty(aChannel);
			const int layerCount = channelProperty.GetSrcObjectCount<FbxLayeredTexture>();
			if (layerCount > 0)
			{
				// We don't use these, hua.
			}
			else
			{
				const int textureCount = channelProperty.GetSrcObjectCount<FbxTexture>();
				if (textureCount > 0)
				{
					// We should only care about the first texture here presumably.
					if (FbxTexture* texture = FbxCast<FbxTexture>(channelProperty.GetSrcObject<FbxTexture>(0)))
					{
						if (const FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture))
						{
							result.Path = fileTexture->GetFileName();
							result.RelativePath = fileTexture->GetRelativeFileName();

							const size_t startPos = result.RelativePath.find_last_of('\\');
							if (startPos != std::string::npos)
								result.Name = result.RelativePath.substr(startPos + 1, result.RelativePath.size() - startPos);
							else
								result.Name = result.RelativePath;
						}
					}
				}
			}

			return result;
		}

		void Internals::GetElementMappingData(FbxLayerElementTemplate<FbxVector4>* anElement, int aFbxContolPointIdx,
			int aPolygonIdx, FbxVector4& outData)
		{
			switch (anElement->GetMappingMode())
			{
			case FbxGeometryElement::eByControlPoint:
				{
					switch (anElement->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						{
							outData.mData[0] = anElement->GetDirectArray().GetAt(aFbxContolPointIdx).mData[0];
							outData.mData[1] = anElement->GetDirectArray().GetAt(aFbxContolPointIdx).mData[1];
							outData.mData[2] = anElement->GetDirectArray().GetAt(aFbxContolPointIdx).mData[2];
						}
						break;
					case FbxGeometryElement::eIndexToDirect:
						{
							const int Idx = anElement->GetIndexArray().GetAt(aFbxContolPointIdx);
							outData.mData[0] = anElement->GetDirectArray().GetAt(Idx).mData[0];
							outData.mData[1] = anElement->GetDirectArray().GetAt(Idx).mData[1];
							outData.mData[2] = anElement->GetDirectArray().GetAt(Idx).mData[2];
						}
						break;
					default:
						throw std::exception("Invalid Reference Mode!");
					}
				}
				break;

			case FbxGeometryElement::eByPolygonVertex:
				{
					switch (anElement->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						{
							outData.mData[0] = anElement->GetDirectArray().GetAt(aPolygonIdx).mData[0];
							outData.mData[1] = anElement->GetDirectArray().GetAt(aPolygonIdx).mData[1];
							outData.mData[2] = anElement->GetDirectArray().GetAt(aPolygonIdx).mData[2];
						}
						break;
					case FbxGeometryElement::eIndexToDirect:
						{
							const int Idx = anElement->GetIndexArray().GetAt(aPolygonIdx);
							outData.mData[0] = anElement->GetDirectArray().GetAt(Idx).mData[0];
							outData.mData[1] = anElement->GetDirectArray().GetAt(Idx).mData[1];
							outData.mData[2] = anElement->GetDirectArray().GetAt(Idx).mData[2];
						}
						break;
					default:
						throw std::exception("Invalid Reference Mode!");
					}
				}
				break;
			}
		}

		void Internals::GetMaterialData(const FbxSurfaceMaterial* aMaterial, Material& outMaterial)
		{
			outMaterial.MaterialName = aMaterial->GetName();
			outMaterial.Ambient = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sAmbient);
			outMaterial.Bump = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sBump);
			outMaterial.Diffuse = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sDiffuse);
			outMaterial.Displacement = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sDisplacementColor);
			outMaterial.Emissive = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sEmissive);
			outMaterial.NormalMap = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sNormalMap);
			outMaterial.Reflection = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sReflection);
			outMaterial.Specular = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sSpecular);
			outMaterial.TransparentColor = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sTransparentColor);
			outMaterial.VectorDisplacement = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sVectorDisplacementColor);
			outMaterial.Shininess = GetMaterialChannelData(aMaterial, FbxSurfaceMaterial::sShininess);
		}

		bool Internals::FbxMeshToMesh(FbxScene* aScene, FbxNode* aMeshNode, Skeleton& aSkeleton,
		                              std::vector<Mesh::Element>& outMeshes, bool bMergeDuplicateVertices)
		{
			const unsigned int numMeshMaterials = aMeshNode->GetMaterialCount();
			outMeshes.reserve(numMeshMaterials);

			std::unordered_multimap<unsigned int, std::pair<size_t, float>> ControlPointWeights;
			float maxExtents[3] = { 0, 0, 0 };
			float minExtents[3] = { 0, 0, 0 };

			FbxMesh* fbxMesh = aMeshNode->GetMesh();

			std::vector<FbxAMatrix> bindPoses{ aSkeleton.Bones.size() };

			if(aSkeleton.GetRoot())
			{
				const FbxVector4 fbxTranslation = aMeshNode->GetGeometricTranslation(FbxNode::eSourcePivot);
				const FbxVector4 fbxRotation = aMeshNode->GetGeometricRotation(FbxNode::eSourcePivot);
				const FbxVector4 fbxScale = aMeshNode->GetGeometricScaling(FbxNode::eSourcePivot);
				const fbxsdk::FbxAMatrix rootTransform = FbxAMatrix(fbxTranslation, fbxRotation, fbxScale);

				for (int deformIdx = 0; deformIdx < fbxMesh->GetDeformerCount(); deformIdx++)
				{
					FbxSkin* fbxSkin = reinterpret_cast<FbxSkin*>(fbxMesh->GetDeformer(deformIdx, FbxDeformer::eSkin));

					// If it's not a skin keep looking.
					if (!fbxSkin)
						continue;

					for (int clusterIdx = 0; clusterIdx < fbxSkin->GetClusterCount(); clusterIdx++)
					{
						// So we go through each cluster.
						FbxCluster* fbxCluster = fbxSkin->GetCluster(clusterIdx);

						// We get the transform of the cluster that was present at skin binding time.
						// This is the "resting pose" if you like.
						fbxsdk::FbxAMatrix meshBindTransform;
						fbxCluster->GetTransformMatrix(meshBindTransform);

						// We also need the link transform. In the case of a Skin it will be the
						// transform to go from local Joint Space to World Space.
						fbxsdk::FbxAMatrix linkTransform;
						fbxCluster->GetTransformLinkMatrix(linkTransform);

						// And finally assemble the Bind Pose Transform.
						// This is the default position of the vertices.
						fbxsdk::FbxAMatrix bindPoseInverseTransform = linkTransform.Inverse() * meshBindTransform * rootTransform;
						fbxsdk::FbxAMatrix bindPose = bindPoseInverseTransform.Inverse();
						// Remember to do this or you will get very interesting results :).
						bindPoseInverseTransform = bindPoseInverseTransform.Transpose();

						// The Link in the skin cluster is the actual joint.
						// Because we already collected all joints we should be able to find it using
						// the acceleration map in the Skeleton.

						std::string jointName = fbxCluster->GetLink()->GetName();
						if(size_t namePos = jointName.find_last_of(':'); namePos != std::string::npos)
						{
							jointName = jointName.substr(namePos + 1);
						}
						size_t jointIndex = aSkeleton.BoneNameToIndex[jointName];

						bindPoses[jointIndex] = bindPoseInverseTransform.Inverse();


						// Store the bind pose on the joint so we can access it later.
						Internals::FBXMatrixToArray(bindPoseInverseTransform, aSkeleton.Bones[jointIndex].BindPoseInverse.Data);

						// Here comes some more control point stuff.
						// We need to collect all the control points that this skin cluster affects.
						// And for those we need to store which joint affects it and its weights.
						for (int i = 0; i < fbxCluster->GetControlPointIndicesCount(); i++)
						{
							const unsigned int c = fbxCluster->GetControlPointIndices()[i];
							const float w = static_cast<float>(fbxCluster->GetControlPointWeights()[i]);
							// This inserts into a multimap.
							// c - control point index.
							// jointIndex - self explanatory.
							// w - the weight for this joint.
							ControlPointWeights.insert({ c, { jointIndex, w } });
						}
					}
				}

				for (size_t i = 0; i < aSkeleton.Bones.size(); i++)
				{
					auto& joint = aSkeleton.Bones[i];

					fbxsdk::FbxAMatrix parentTransform;
					if (joint.ParentIdx >= 0)
					{
						parentTransform = bindPoses[joint.ParentIdx];
					}

					const fbxsdk::FbxAMatrix localPose = parentTransform.Inverse() * bindPoses[i];

					joint.restPosition = Internals::FbxVec4ToVec3(localPose.GetT());
					joint.restRotation = Internals::FbxQuatToQuat(localPose.GetQ());
					joint.restScale = Internals::FbxVec4ToVec3(localPose.GetS());
				}
			}

			FbxAMatrix tempmatrix = aMeshNode->EvaluateGlobalTransform();
			FbxTransform tempxform = aMeshNode->GetTransform();

			// If we have materials we may need to cut this mesh up into multiple.
			const bool bHasMaterials = aMeshNode->GetMaterialCount() != 0;
			FbxSurfaceMaterial* currentSceneMaterial = nullptr;

			// We need to load per material index since each mesh can only have one material when rendering.
			// This means that if it has more than one they will be separated into two meshes.
			for (int meshMaterialIndex = 0; meshMaterialIndex < aMeshNode->GetMaterialCount() || meshMaterialIndex == 0; meshMaterialIndex++)
			{
				Mesh::Element currentMeshData = {};
				currentMeshData.MeshName = aMeshNode->GetName();

				const auto fbxTranslation = aMeshNode->LclTranslation.Get();
				const auto fbxRotation = aMeshNode->LclRotation.Get();
				const auto fbxScale = aMeshNode->LclScaling.Get();

				currentMeshData.localTranslation = { (float)fbxTranslation[0], (float)fbxTranslation[1], (float)fbxTranslation[2] };
				currentMeshData.localRotation = { (float)fbxRotation[0], (float)fbxRotation[1], (float)fbxRotation[2] };
				currentMeshData.localScale = { (float)fbxScale[0], (float)fbxScale[1], (float)fbxScale[2] };

				if (bHasMaterials)
				{
					for (int sceneMaterialIndex = 0; sceneMaterialIndex < aScene->GetMaterialCount(); sceneMaterialIndex++)
					{
						FbxSurfaceMaterial* sceneMaterial = aScene->GetMaterial(sceneMaterialIndex);
						FbxSurfaceMaterial* meshNodeMaterial = aMeshNode->GetMaterial(meshMaterialIndex);
						if (sceneMaterial == meshNodeMaterial)
						{
							currentSceneMaterial = sceneMaterial;
							currentMeshData.MaterialIndex = sceneMaterialIndex;
							break;
						}
					}
				}
				else
				{
					currentMeshData.MaterialIndex = 0;
				}
				
				FbxLayerElementMaterial* fbxElementMaterial = fbxMesh->GetElementMaterial();

				const int fbxMeshPolygonCount = fbxMesh->GetPolygonCount();

				currentMeshData.Vertices.reserve(static_cast<size_t>(fbxMeshPolygonCount) * 3ULL);
				currentMeshData.Indices.reserve(currentMeshData.Vertices.capacity());

				std::unordered_map<size_t, VertexDuplicateData> VertexDuplicateAccelMap;
				VertexDuplicateAccelMap.reserve(currentMeshData.Vertices.capacity());

				unsigned int IndexCounter = 0;
				for (int p = 0; p < fbxMeshPolygonCount; p++)
				{
					if (bHasMaterials)
					{
						// This is the index of the materials in the mesh element array.
						// It doesn't always correspond to the scene material list since the first
						// material here might be material n in the scene.
						const int IndexAtP = fbxElementMaterial->GetIndexArray().GetAt(p);
						FbxSurfaceMaterial* polygonMaterial = aMeshNode->GetMaterial(IndexAtP);
						if (currentSceneMaterial != polygonMaterial)
						{
							continue;
						}
					}

					for (int v = 0; v < 3; v++)
					{
						const int fbxControlPtIndex = fbxMesh->GetPolygonVertex(p, v);
						const FbxVector4 fbxVxPos = fbxMesh->GetControlPointAt(fbxControlPtIndex);
						FbxVector2 fbxVxUVs[4];
						const int fbxNumUVs = fbxMesh->GetElementUVCount();

						const int fbxTextureUVIndex = fbxMesh->GetTextureUVIndex(p, v);
						for (int uv = 0; uv < fbxNumUVs && uv < 4; uv++)
						{
							FbxGeometryElementUV* fbxUvElement = fbxMesh->GetElementUV(uv);
							fbxVxUVs[uv] = fbxUvElement->GetDirectArray().GetAt(fbxTextureUVIndex);
						}

						const int polygonIndex = p * 3 + v;

						FbxVector4 fbxNormal;
						FbxGeometryElementNormal* fbxNormalElement = fbxMesh->GetElementNormal(0);
						GetElementMappingData(fbxNormalElement, fbxControlPtIndex, polygonIndex, fbxNormal);

						FbxVector4 fbxTangent;
						FbxGeometryElementTangent* fbxTangentElement = fbxMesh->GetElementTangent(0);
						GetElementMappingData(fbxTangentElement, fbxControlPtIndex, polygonIndex, fbxTangent);

						FbxVector4 fbxBinormal;
						FbxGeometryElementBinormal* fbxBinormalElement = fbxMesh->GetElementBinormal(0);
						GetElementMappingData(fbxBinormalElement, fbxControlPtIndex, polygonIndex, fbxBinormal);

						// BUG: FBX SDK 2020 doesn't seem to handle vx color winding correctly in certain
						// circumstances. It seems to work in eByVertex-eDirect but not eIndexToDirect?
						// Needs more testing.

						int windCorrection;
						switch (polygonIndex)
						{
						case 1:
							windCorrection = 2;
							break;
						case 2:
							windCorrection = 1;
							break;
						default:
							windCorrection = polygonIndex;
							break;
						}

						//const int windedPolygonIndex = p * 3 + windCorrection;
						const int windedPolygonIndex = polygonIndex;

						FbxColor fbxColors[4];
						const int fbxNumVxColorChannels = fbxMesh->GetElementVertexColorCount();
						for (int col = 0; col < fbxNumVxColorChannels && col < 4; col++)
						{
							FbxGeometryElementVertexColor* colElement = fbxMesh->GetElementVertexColor(col);
							switch (colElement->GetMappingMode())
							{
							case FbxGeometryElement::eByControlPoint:
							{
								switch (fbxNormalElement->GetReferenceMode())
								{
								case FbxGeometryElement::eDirect:
								{
									fbxColors[col] = colElement->GetDirectArray().GetAt(fbxControlPtIndex);
								}
								break;
								case FbxGeometryElement::eIndexToDirect:
								{
									const int Idx = colElement->GetIndexArray().GetAt(fbxControlPtIndex);
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
								// DB 221006 Possible that this works as intended without having to use
								// the corrected winding? Needs testing.
								switch (colElement->GetReferenceMode())
								{
								case FbxGeometryElement::eDirect:
								{
									fbxColors[col] = colElement->GetDirectArray().GetAt(windedPolygonIndex);
								}
								break;
								case FbxGeometryElement::eIndexToDirect:
								{
									const int Idx = colElement->GetIndexArray().GetAt(windedPolygonIndex);
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

						Vertex vx;
						vx.Position[0] = static_cast<float>(fbxVxPos[0]);
						vx.Position[1] = static_cast<float>(fbxVxPos[1]);
						vx.Position[2] = static_cast<float>(fbxVxPos[2]);
						vx.Position[3] = static_cast<float>(fbxVxPos[3]);

						for(int uv = 0; uv < fbxNumUVs; uv++)
						{
							vx.UVs[uv][0] = static_cast<float>(fbxVxUVs[uv][0]);
							vx.UVs[uv][1] = 1 - static_cast<float>(fbxVxUVs[uv][1]);
						}

						vx.Normal[0] = static_cast<float>(fbxNormal[0]);
						vx.Normal[1] = static_cast<float>(fbxNormal[1]);
						vx.Normal[2] = static_cast<float>(fbxNormal[2]);

						vx.Tangent[0] = static_cast<float>(fbxTangent[0]);
						vx.Tangent[1] = static_cast<float>(fbxTangent[1]);
						vx.Tangent[2] = static_cast<float>(fbxTangent[2]);

						vx.BiNormal[0] = static_cast<float>(fbxBinormal[0]);
						vx.BiNormal[1] = static_cast<float>(fbxBinormal[1]);
						vx.BiNormal[2] = static_cast<float>(fbxBinormal[2]);

						for (int vxColSet = 0; vxColSet < fbxNumVxColorChannels; vxColSet++)
						{
							vx.VertexColors[vxColSet][0] = static_cast<float>(fbxColors[vxColSet][0]);
							vx.VertexColors[vxColSet][1] = static_cast<float>(fbxColors[vxColSet][1]);
							vx.VertexColors[vxColSet][2] = static_cast<float>(fbxColors[vxColSet][2]);
							vx.VertexColors[vxColSet][3] = static_cast<float>(fbxColors[vxColSet][3]);
						}

						if (vx.Position[0] > maxExtents[0])
							maxExtents[0] = vx.Position[0];
						if (vx.Position[1] > maxExtents[1])
							maxExtents[1] = vx.Position[1];
						if (vx.Position[2] > maxExtents[2])
							maxExtents[2] = vx.Position[2];

						if (vx.Position[0] < minExtents[0])
							minExtents[0] = vx.Position[0];
						if (vx.Position[1] < minExtents[1])
							minExtents[1] = vx.Position[1];
						if (vx.Position[2] < minExtents[2])
							minExtents[2] = vx.Position[2];

						unsigned int BoneIDs[] = { 0, 0, 0, 0 };
						float BoneWeights[] = { 0, 0, 0, 0 };

						if (aSkeleton.GetRoot())
						{
							// Since we're making a bit of a complex iteration we need to define the iterator.
							// It's a lot less to type that way.
							typedef std::unordered_multimap<unsigned int, std::pair<size_t, float>>::iterator MMIter;

							// Then we use equal range to get all the data stored for this specific control point.
							std::pair<MMIter, MMIter> values = ControlPointWeights.equal_range(fbxControlPtIndex);

							// This idx is to loop on the 4 indices of ID and Weight.
							int idx = 0;
							for (MMIter it = values.first; it != values.second && idx < 4; ++it)
							{
								std::pair<size_t, float> BoneAndWeight = it->second;
								BoneIDs[idx] = static_cast<unsigned>(BoneAndWeight.first);
								BoneWeights[idx] = BoneAndWeight.second;
								idx++;
							}
						}

						vx.BoneIDs[0] = BoneIDs[0];
						vx.BoneIDs[1] = BoneIDs[1];
						vx.BoneIDs[2] = BoneIDs[2];
						vx.BoneIDs[3] = BoneIDs[3];

						vx.BoneWeights[0] = BoneWeights[0];
						vx.BoneWeights[1] = BoneWeights[1];
						vx.BoneWeights[2] = BoneWeights[2];
						vx.BoneWeights[3] = BoneWeights[3];

						if(bMergeDuplicateVertices)
						{
							// A drawback of using control points is that we MAY get duplicate vertices.
							// This means we'll need to compare and ensure that it is a unique vert.
							VertexHash Hasher;
							size_t hash = Hasher(vx);
							if (VertexDuplicateAccelMap.find(hash) == VertexDuplicateAccelMap.end())
							{
								VertexDuplicateAccelMap[hash] = { /*{ vx },*/ IndexCounter, hash };
								currentMeshData.Vertices.push_back(vx);
								currentMeshData.Indices.push_back(IndexCounter++);
							}
							else
							{
								currentMeshData.Indices.push_back(VertexDuplicateAccelMap[hash].Idx);
							}
						}
						else
						{
							currentMeshData.Vertices.push_back(vx);
							currentMeshData.Indices.push_back(IndexCounter++);
						}
					}
				}

				if(!currentMeshData.Vertices.empty())
				{
					float extentsCenter[3];
					extentsCenter[0] = 0.5f * (minExtents[0] + maxExtents[0]);
					extentsCenter[1] = 0.5f * (minExtents[1] + maxExtents[1]);
					extentsCenter[2] = 0.5f * (minExtents[2] + maxExtents[2]);

					float boxExtents[3];
					boxExtents[0] = 0.5f * (maxExtents[0] - minExtents[0]);
					boxExtents[1] = 0.5f * (maxExtents[1] - minExtents[1]);
					boxExtents[2] = 0.5f * (maxExtents[2] - minExtents[2]);

					float extentRadius = std::max(boxExtents[0], std::max(boxExtents[1], boxExtents[2]));

					currentMeshData.BoxSphereBounds = {
						{boxExtents[0], boxExtents[1], boxExtents[2]},
						{extentsCenter[0], extentsCenter[1], extentsCenter[2]},
						extentRadius
					};

					currentMeshData.BoxBounds = Box::FromAABB(currentMeshData.BoxSphereBounds.Center, currentMeshData.BoxSphereBounds.BoxExtents);

					outMeshes.push_back(currentMeshData);
				}

				VertexDuplicateAccelMap.clear();

			}

			ControlPointWeights.clear();

			return true;
		}
	}
}




