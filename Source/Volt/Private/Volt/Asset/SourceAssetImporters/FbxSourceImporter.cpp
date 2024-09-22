#include "vtpch.h"

#include "Volt/Asset/SourceAssetImporters/FbxSourceImporter.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Rendering/Mesh/MeshCommon.h"

#include <AssetSystem/AssetManager.h>

#include <fbxsdk.h>

VT_DEFINE_LOG_CATEGORY(LogFbxSourceImporter);

using namespace fbxsdk;

namespace Volt
{
	namespace FbxUtility
	{
		glm::vec3 ToVec3(const FbxVector4& v)
		{
			return { v[0], v[1], v[2] };
		}

		glm::vec2 ToVec2(const FbxVector2& v)
		{
			return { v[0], v[1] };
		}
	}

	struct FbxSDKDeleter
	{
		template<typename T>
		void operator()(T* object) const
		{
			object->Destroy();
		}
	};

	struct FbxVertex
	{
		int32_t material;
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec2 texCoords;
	};

	namespace ElementType
	{
		constexpr uint32_t Material = 0;
		constexpr uint32_t Position = 1;
		constexpr uint32_t Normal = 2;
		constexpr uint32_t UV = 3;
		constexpr uint32_t Tangent = 4;
		constexpr uint32_t Count = 5;
	}

	struct FatIndex
	{
		int32_t elements[ElementType::Count];

		FatIndex()
		{
			for (int i = 0; i < ElementType::Count; i++)
			{
				elements[i] = -1;
			}
		}
	};

	using FbxScenePtr = std::unique_ptr<FbxScene, FbxSDKDeleter>;
	using FbxManagerPtr = std::unique_ptr<FbxManager, FbxSDKDeleter>;
	using FbxIOSettingsPtr = std::unique_ptr<FbxIOSettings, FbxSDKDeleter>;
	using FbxImporterPtr = std::unique_ptr<fbxsdk::FbxImporter, FbxSDKDeleter>;

	inline void SetupIOSettings(FbxIOSettings& ioSettings, const MeshSourceImportConfig& importConfig, const std::filesystem::path& filepath)
	{
		if (!importConfig.password.empty())
		{
			ioSettings.SetStringProp(IMP_FBX_PASSWORD, importConfig.password.c_str());
			ioSettings.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);
		}

		// General
		ioSettings.SetBoolProp(IMP_PRESETS, true);
		ioSettings.SetBoolProp(IMP_STATISTICS, true);
		ioSettings.SetBoolProp(IMP_GEOMETRY, true);
		ioSettings.SetBoolProp(IMP_ANIMATION, true);
		ioSettings.SetBoolProp(IMP_LIGHT, false);
		ioSettings.SetBoolProp(IMP_ENVIRONMENT, false);
		ioSettings.SetBoolProp(IMP_CAMERA, false);
		ioSettings.SetBoolProp(IMP_VIEW_CUBE, false);
		ioSettings.SetBoolProp(IMP_ZOOMEXTENTS, false);
		ioSettings.SetBoolProp(IMP_GLOBAL_AMBIENT_COLOR, false);
		ioSettings.SetBoolProp(IMP_META_DATA, false);
		ioSettings.SetBoolProp(IMP_REMOVEBADPOLYSFROMMESH, false);

		// FBX
		ioSettings.SetBoolProp(IMP_FBX_TEMPLATE, true);
		ioSettings.SetBoolProp(IMP_FBX_PIVOT, true);
		ioSettings.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
		ioSettings.SetBoolProp(IMP_FBX_CHARACTER, false);
		ioSettings.SetBoolProp(IMP_FBX_CONSTRAINT, false);
		ioSettings.SetBoolProp(IMP_FBX_MERGE_LAYER_AND_TIMEWARP, false);
		ioSettings.SetBoolProp(IMP_FBX_GOBO, false);
		ioSettings.SetBoolProp(IMP_FBX_SHAPE, false);
		ioSettings.SetBoolProp(IMP_FBX_LINK, false);
		ioSettings.SetBoolProp(IMP_FBX_MATERIAL, true);
		ioSettings.SetBoolProp(IMP_FBX_TEXTURE, true);
		ioSettings.SetBoolProp(IMP_FBX_MODEL, true);
		ioSettings.SetBoolProp(IMP_FBX_ANIMATION, true);
		ioSettings.SetBoolProp(IMP_FBX_EXTRACT_EMBEDDED_DATA, true);

		ioSettings.SetStringProp(IMP_EXTRACT_FOLDER, filepath.parent_path().string().c_str());

		// 3DS
		ioSettings.SetBoolProp(IMP_3DS_REFERENCENODE, false);
		ioSettings.SetBoolProp(IMP_3DS_TEXTURE, true);
		ioSettings.SetBoolProp(IMP_3DS_MATERIAL, true);
		ioSettings.SetBoolProp(IMP_3DS_ANIMATION, false);
		ioSettings.SetBoolProp(IMP_3DS_MESH, true);
		ioSettings.SetBoolProp(IMP_3DS_LIGHT, false);
		ioSettings.SetBoolProp(IMP_3DS_CAMERA, false);
		ioSettings.SetBoolProp(IMP_3DS_AMBIENT_LIGHT, false);
		ioSettings.SetBoolProp(IMP_3DS_RESCALING, true);
		ioSettings.SetBoolProp(IMP_3DS_FILTER, false);
		ioSettings.SetBoolProp(IMP_3DS_SMOOTHGROUP, false);

		// OBJ
		ioSettings.SetBoolProp(IMP_OBJ_REFERENCE_NODE, false);

		// DXF
		ioSettings.SetBoolProp(IMP_DXF_WELD_VERTICES, false);
		ioSettings.SetBoolProp(IMP_DXF_OBJECT_DERIVATION, false);
		ioSettings.SetBoolProp(IMP_DXF_REFERENCE_NODE, false);
	}

	inline std::string GetFbxNodePath(FbxNode* node)
	{
		constexpr std::string_view rootName = "<root>";

		std::string result;
		if (node != nullptr && node->GetParent() != nullptr)
		{
			result = rootName;
		}

		while (node != nullptr && node->GetParent() != nullptr)
		{
			std::string nodeDesc;

			const char* const nodeName = node->GetName();
			if (nodeName && nodeName[0])
			{
				nodeDesc = nodeName;
			}
			else
			{
				int nodeIndex = -1;
				const FbxNode* const parentNode = node->GetParent();
				if (parentNode)
				{
					const int childCount = parentNode->GetChildCount();
					for (int i = 0; i < childCount; i++)
					{
						if (parentNode->GetChild(i) == node)
						{
							nodeIndex = i;
							break;
						}
					}
				}

				if (nodeIndex >= 0)
				{
					nodeDesc = std::format("<node at index {}>", nodeIndex);
				}
				else
				{
					nodeDesc = "<anonymous node>";
				}
			}

			if (result.empty())
			{
				result.swap(nodeDesc);
			}
			else
			{
				result = nodeDesc + "/" + result;
			}

			node = node->GetParent();
		}

		if (result.empty())
		{
			result = "<null>";
		}

		return result;
	}

	template<typename Functor>
	inline void VisitNodes(FbxScene* scene, Functor& functor)
	{
		auto visitor = [&](FbxNode* parent, FbxNode* node, auto visitor) -> bool
		{
			if (node)
			{
				const int childCount = node->GetChildCount();
				functor(parent, node);

				for (int i = 0; i < childCount; i++)
				{
					visitor(node, node->GetChild(i), visitor);
				}
			}

			return false;
		};

		visitor(nullptr, scene->GetRootNode(), visitor);
	}

	template<typename Attribute, typename Functor>
	inline void VisitNodeAttributesOfType(FbxScene* scene, Functor& functor, bool primaryAttribsOnly = false)
	{
		auto selector = [&](FbxNodeAttribute* attrib, FbxNode* node)
		{
			if (attrib && attrib->Is<Attribute>())
			{
				functor(static_cast<Attribute*>(attrib), node);
			}

			return true;
		};

		auto visitor = [&](FbxNode* parent, FbxNode* node)
		{
			if (primaryAttribsOnly)
			{
				selector(node->GetNodeAttribute(), node);
			}
			else
			{
				const int attribCount = node->GetNodeAttributeCount();
				for (int i = 0; i < attribCount; i++)
				{
					selector(node->GetNodeAttributeByIndex(i), node);
				}
			}

			return true;
		};

		VisitNodes(scene, visitor);
	}

	inline void PostProccessFbxScene(FbxScene* fbxScene, const MeshSourceImportConfig& importConfig, const SourceAssetUserImportData& userData)
	{
		FbxGeometryConverter converter(fbxScene->GetFbxManager());

		if (importConfig.removeDegeneratePolygons)
		{
			FbxArray<FbxNode*> removedFromNodes;
			converter.RemoveBadPolygonsFromMeshes(fbxScene, &removedFromNodes);

			const int32_t nodeCount = removedFromNodes.Size();
			for (int i = 0; i < nodeCount; i++)
			{
				const std::string nodeName = GetFbxNodePath(removedFromNodes.GetAt(i));
				userData.OnInfo(std::format("One or more meshes at node {} had degenerate polygons removed!", nodeName));
			}
		}

		if (importConfig.triangulate)
		{
			Vector<FbxGeometry*> geomToTriangulate;
			auto findGeomToTriangulate = [&](FbxGeometry* geometry, FbxNode* node)
			{
				VT_ENSURE(geometry);

				if (FbxMesh* mesh = FbxCast<FbxMesh>(geometry))
				{
					if (mesh->IsTriangleMesh())
					{
						return;
					}
				}
				else if (!geometry->Is<FbxNurbsCurve>() && !geometry->Is<FbxNurbsSurface>() && !geometry->Is<FbxPatch>())
				{
					return; // Non convertiable
				}

				geomToTriangulate.emplace_back(geometry);
			};

			VisitNodeAttributesOfType<FbxGeometry>(fbxScene, findGeomToTriangulate);

			if (!geomToTriangulate.empty())
			{
				for (auto* fbxGeom : geomToTriangulate)
				{
					std::string geomName = fbxGeom->GetName();
					if (geomName.empty())
					{
						const FbxNode* node = fbxGeom->GetNode();
						geomName = node ? node->GetName() : "";

						if (geomName.empty())
						{
							geomName = "<anonymous>";
						}
					}

					const bool result = converter.Triangulate(fbxGeom, true) != nullptr;
					if (!result)
					{
						userData.OnWarning(std::format("Failed to triangulate {}!", geomName));
					}
				}
			}
		}

		if (importConfig.generateNormalsMode != GenerateNormalsMode::Never)
		{
			Vector<FbxMesh*> geomToGenerate;
			const bool overwrite = (importConfig.generateNormalsMode == GenerateNormalsMode::OverwriteSmooth) || (importConfig.generateNormalsMode == GenerateNormalsMode::OverwriteHard);

			auto generator = [&](FbxMesh* mesh, FbxNode* node)
			{
				if (!overwrite)
				{
					const int layerCount = mesh->GetLayerCount();
					for (int i = 0; i < layerCount; i++)
					{
						FbxLayer* fbxLayer = mesh->GetLayer(i);
						if (fbxLayer && fbxLayer->GetNormals())
						{
							return;
						}
					}
				}
				geomToGenerate.emplace_back(mesh);
			};

			VisitNodeAttributesOfType<FbxMesh>(fbxScene, generator);

			if (!geomToGenerate.empty())
			{
				const bool smoothNormals = (importConfig.generateNormalsMode == GenerateNormalsMode::Smooth) || (importConfig.generateNormalsMode == GenerateNormalsMode::OverwriteSmooth);
				const bool clockwise = false;

				for (auto* fbxMesh : geomToGenerate)
				{
					std::string meshName = fbxMesh->GetName();
					if (meshName.empty())
					{
						const FbxNode* node = fbxMesh->GetNode();
						meshName = node ? node->GetName() : "";

						if (meshName.empty())
						{
							meshName = "<anonymous>";
						}
					}

					const bool result = fbxMesh->GenerateNormals(true, smoothNormals, clockwise);
					if (!result)
					{
						userData.OnWarning(std::format("Failed to generate {} normals for {}", (smoothNormals ? "smooth" : "hard"), meshName));
					}
				}
			}
		}

		if (importConfig.generateTangents)
		{
			Vector<FbxMesh*> meshesToGenerate;
			auto generator = [&](FbxMesh* mesh, FbxNode* node)
			{
				const int layerCount = mesh->GetLayerCount();
				for (int i = 0; i < layerCount; i++)
				{
					FbxLayer* fbxLayer = mesh->GetLayer(i);
					if (fbxLayer && fbxLayer->GetTangents() && fbxLayer->GetBinormals())
					{
						return;
					}
				}
				meshesToGenerate.emplace_back(mesh);
			};

			VisitNodeAttributesOfType<FbxMesh>(fbxScene, generator);

			for (auto* fbxMesh : meshesToGenerate)
			{
				std::string meshName = fbxMesh->GetName();
				if (meshName.empty())
				{
					const FbxNode* node = fbxMesh->GetNode();
					meshName = node ? node->GetName() : "";

					if (meshName.empty())
					{
						meshName = "<anonymous>";
					}
				}

				if (!fbxMesh->GenerateTangentsData(0, true, false))
				{
					userData.OnWarning(std::format("Failed to generate tangents/binormals for {}", meshName));
				}
			}
		}

		if (importConfig.convertScene)
		{
			fbxsdk::FbxAxisSystem axisSystem(fbxsdk::FbxAxisSystem::DirectX);
			axisSystem.DeepConvertScene(fbxScene);
		}
	}

	inline void SetElementIndices(Vector<FatIndex>& fatIndices, int32_t elementType, const FbxLayerElement* layerElement, const FbxLayerElementArrayTemplate<int32_t>& indexArray, const FbxMesh& mesh)
	{
		const int32_t indexCount = static_cast<int32_t>(fatIndices.size());

		if (layerElement->GetMappingMode() == FbxLayerElement::eByControlPoint)
		{
			if (layerElement->GetReferenceMode() == FbxLayerElement::eDirect)
			{
				for (int32_t i = 0; i < indexCount; i++)
				{
					fatIndices[i].elements[elementType] = fatIndices[i].elements[ElementType::Position];
				}
			}
			else if (layerElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
			{
				for (int32_t i = 0; i < indexCount; i++)
				{
					fatIndices[i].elements[elementType] = indexArray.GetAt(fatIndices[i].elements[ElementType::Position]);
				}
			}
		}
		else if (layerElement->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
		{
			if (layerElement->GetReferenceMode() == FbxLayerElement::eDirect)
			{
				for (int32_t i = 0; i < indexCount; i++)
				{
					fatIndices[i].elements[elementType] = i;
				}
			}
			else if (layerElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
			{
				for (int32_t i = 0; i < indexCount; i++)
				{
					fatIndices[i].elements[elementType] = indexArray.GetAt(i);
				}
			}
		}
		else if (layerElement->GetMappingMode() == FbxLayerElement::eAllSame)
		{
			if (layerElement->GetReferenceMode() == FbxLayerElement::eDirect)
			{
				for (int32_t i = 0; i < indexCount; i++)
				{
					fatIndices[i].elements[elementType] = 0;
				}
			}
			else if (layerElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
			{
				for (int32_t i = 0; i < indexCount; i++)
				{
					fatIndices[i].elements[elementType] = indexArray.GetAt(0);
				}
			}
		}
		else if (layerElement->GetMappingMode() == FbxLayerElement::eByPolygon)
		{
			const int32_t faceCount = mesh.GetPolygonCount();

			if (layerElement->GetReferenceMode() == FbxLayerElement::eDirect)
			{
				int32_t cursor = 0;
				for (int32_t i = 0; i < faceCount; i++)
				{
					for (int32_t j = 0; j < mesh.GetPolygonSize(i); j++)
					{
						fatIndices[cursor++].elements[elementType] = i;
					}
				}
			}
			else if (layerElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
			{
				int32_t cursor = 0;
				for (int32_t i = 0; i < faceCount; i++)
				{
					for (int32_t j = 0; j < mesh.GetPolygonSize(i); j++)
					{
						fatIndices[cursor++].elements[elementType] = indexArray.GetAt(i);
					}
				}
			}
		}
		else
		{
			VT_ASSERT(false);
		}
	}

	inline void TriangulateMesh(const FbxMesh& fbxMesh, const Vector<FatIndex>& input, Vector<FatIndex>& output)
	{
		const int32_t faceCount = fbxMesh.GetPolygonCount();
		int32_t t0 = 0;

		for (int32_t i = 0; i < faceCount; i++)
		{
			const int32_t faceSize = fbxMesh.GetPolygonSize(i);
			int32_t t1 = t0 + 1;
			for (int32_t j = 2; j < faceSize; j++)
			{
				output.emplace_back(input[t0]);
				output.emplace_back(input[t1]);
				output.emplace_back(input[t1 + 1]);
				t1++;
			}

			t0 += faceSize;
		}
	}

	inline void CreateNonIndexedMesh(const FbxMesh& fbxMesh, Vector<FbxVertex>& outVertices)
	{
		VT_ENSURE(outVertices.empty());

		const int32_t indexCount = fbxMesh.GetPolygonVertexCount();
		const int32_t faceCount = fbxMesh.GetPolygonCount();

		// We only care about layer 0
		const FbxLayer* const layer0 = fbxMesh.GetLayer(0);

		Vector<FatIndex> fatIndices(indexCount);

		// Set position indices
		{
			int32_t cursor = 0;
			for (int32_t i = 0; i < faceCount; ++i)
			{
				for (int32_t j = 0; j < fbxMesh.GetPolygonSize(i); j++)
				{
					fatIndices[cursor].elements[ElementType::Position] = fbxMesh.GetPolygonVertex(i, j);
					cursor++;
				}
			}
		}

		// Set normals
		const FbxLayerElementNormal* const inputNormals = layer0 ? layer0->GetNormals() : nullptr;
		const bool hasNormals = inputNormals != nullptr;
		if (hasNormals)
		{
			SetElementIndices(fatIndices, ElementType::Normal, inputNormals, inputNormals->GetIndexArray(), fbxMesh);
		}
		else
		{
			VT_LOGC(Warning, LogFbxSourceImporter, "Mesh is missing normals, setting to zero vector!");
		}

		// Set tangents
		const FbxLayerElementTangent* const inputTangents = layer0 ? layer0->GetTangents() : nullptr;
		const bool hasTangents = inputTangents != nullptr;
		if (hasTangents)
		{
			SetElementIndices(fatIndices, ElementType::Tangent, inputTangents, inputTangents->GetIndexArray(), fbxMesh);
		}
		else
		{
			VT_LOGC(Warning, LogFbxSourceImporter, "Mesh is missing tangents, setting to zero vector!");
		}

		// Set UVs
		const bool hasTexCoords = layer0 && layer0->GetUVSetCount() > 0;
		if (hasTexCoords)
		{
			const FbxLayerElementUV* const inputTexCoords = layer0->GetUVSets().GetFirst();
			SetElementIndices(fatIndices, ElementType::UV, inputTexCoords, inputTexCoords->GetIndexArray(), fbxMesh);
		}

		// Set materials
		const bool hasMaterials = layer0 && layer0->GetMaterials() != nullptr;
		if (hasMaterials)
		{
			const FbxLayerElementMaterial* const inputMaterials = layer0->GetMaterials();
			SetElementIndices(fatIndices, ElementType::Material, inputMaterials, inputMaterials->GetIndexArray(), fbxMesh);

			// https://help.autodesk.com/view/FBX/2016/ENU/?guid=__cpp_ref_class_fbx_layer_element_material_html
			VT_ENSURE(inputMaterials->GetReferenceMode() == FbxLayerElement::eIndexToDirect);
		}

		Vector<FatIndex> triangles;
		TriangulateMesh(fbxMesh, fatIndices, triangles);
		fatIndices.swap(triangles);

		const size_t triIndexCount = fatIndices.size();

		outVertices.resize(triIndexCount);
		for (int32_t i = 0; i < triIndexCount; i++)
		{
			if (hasMaterials)
			{
				outVertices[i].material = fatIndices[i].elements[ElementType::Material];
			}
			else
			{
				outVertices[i].material = -1;
			}

			outVertices[i].position = FbxUtility::ToVec3(fbxMesh.GetControlPointAt(fatIndices[i].elements[ElementType::Position]));

			if (hasNormals)
			{
				outVertices[i].normal = FbxUtility::ToVec3(inputNormals->GetDirectArray().GetAt(fatIndices[i].elements[ElementType::Normal]));
			}
			else
			{
				outVertices[i].normal = 0.f;
			}

			if (hasTangents)
			{
				outVertices[i].tangent = FbxUtility::ToVec3(inputTangents->GetDirectArray().GetAt(fatIndices[i].elements[ElementType::Tangent]));
			}
			else
			{
				outVertices[i].tangent = 0.f;
			}

			if (hasTexCoords)
			{
				const FbxLayerElementUV* const inputTexCoords = layer0->GetUVSets().GetFirst();
				outVertices[i].texCoords = FbxUtility::ToVec2(inputTexCoords->GetDirectArray().GetAt(fatIndices[i].elements[ElementType::UV]));
			}
		}

		// Set default tex coords
		if (!hasTexCoords)
		{
			for (size_t i = 0; i < triIndexCount; i += 3)
			{
				outVertices[i + 0].texCoords = { 0.f, 0.f };
				outVertices[i + 1].texCoords = { 1.f, 0.f };
				outVertices[i + 2].texCoords = { 1.f, 1.f };
			}
		}

		// Make sure that all the vertices in the same triangle has the same material index
		for (size_t i = 0; i < triIndexCount; i += 3)
		{
			outVertices[i + 2].material = outVertices[i + 1].material = outVertices[i].material;
		}
	}

	inline Vector<Ref<Material>> CreateSceneMaterials(FbxScene* fbxScene, const MeshSourceImportConfig& importConfig)
	{
		FbxArray<FbxSurfaceMaterial*> sceneMaterials;
		fbxScene->FillMaterialArray(sceneMaterials);

		Vector<Ref<Material>> result;

		for (int32_t i = 0; i < sceneMaterials.Size(); i++)
		{
			FbxSurfaceMaterial* fbxMaterial = sceneMaterials[i];
			const char* const name = fbxMaterial->GetName();

			auto it = std::find_if(result.begin(), result.end(), [name](const auto mat) 
			{
				return mat->GetName() == name;
			});

			if (it != result.end())
			{
				VT_LOGC(Warning, LogFbxSourceImporter, "Ignoring material with duplicated name '{}'!", name);
				continue;
			}

			Ref<Material> material = AssetManager::CreateAsset<Material>(importConfig.destinationDirectory, name);
			result.emplace_back(material);
		}

		// Create a dummy material
		if (result.empty())
		{
			Ref<Material> material = AssetManager::CreateAsset<Material>(importConfig.destinationDirectory, importConfig.destinationFilename + "_DummyMat");
			result.emplace_back(material);
		}

		return result;
	}

	inline void TranslateNodeToSceneMaterials(const FbxNode* fbxNode, const Vector<Ref<Material>>& materials, Vector<FbxVertex>& vertices)
	{
		const int32_t nodeMaterialCount = fbxNode->GetMaterialCount();

		Vector<int32_t> materialMap(nodeMaterialCount, -1);

		for (size_t i = 0; i < vertices.size(); i++)
		{
			int32_t materialIndex = 0;
			if (vertices[i].material >= 0 && vertices[i].material < nodeMaterialCount)
			{
				if (materialMap[vertices[i].material] < 0)
				{
					const FbxSurfaceMaterial* const fbxMaterial = fbxNode->GetMaterial(vertices[i].material);
					if (fbxMaterial)
					{
						int32_t j = 0;
						while (j < static_cast<int32_t>(materials.size()))
						{
							if (fbxMaterial->GetName() == materials[j]->GetName())
							{
								break;
							}

							j++;
						}

						VT_ENSURE(j < materials.size());
						materialMap[vertices[i].material] = j;

					}
					else
					{
						materialMap[vertices[i].material] = 0;
					}
				}

				materialIndex = materialMap[vertices[i].material];
			}

			vertices[i].material = materialIndex;
		}
	}

	// From: https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
	inline bool AlmostEqual(float lhs, float rhs)
	{
		const float diff = fabs(lhs - rhs);
		lhs = fabs(lhs);
		rhs = fabs(rhs);

		float largest = (rhs > lhs) ? rhs : lhs;
		return (diff <= largest * FLT_EPSILON);
	}

	inline bool AlmostEqual(const glm::vec3& lhs, const glm::vec3& rhs)
	{
		return
			AlmostEqual(lhs.x, rhs.x) &&
			AlmostEqual(lhs.y, rhs.y) &&
			AlmostEqual(lhs.z, rhs.z);
	}

	inline bool AlmostEqual(const glm::vec2& lhs, const glm::vec2& rhs)
	{
		return
			AlmostEqual(lhs.x, rhs.x) &&
			AlmostEqual(lhs.y, rhs.y);
	}

	inline bool AlmostEqual(const FbxVertex& lhs, const FbxVertex& rhs)
	{
		return
			lhs.material == rhs.material &&
			AlmostEqual(lhs.position, rhs.position) &&
			AlmostEqual(lhs.normal, rhs.normal) &&
			AlmostEqual(lhs.tangent, rhs.tangent) &&
			AlmostEqual(lhs.texCoords, rhs.texCoords);
	}

	void FbxSourceImporter::CreateSubMeshFromVertexRange(Ref<Mesh> destinationMesh, const FbxVertex* vertices, size_t indexCount, const std::string& name)
	{
		constexpr uint32_t MaxUInt = std::numeric_limits<uint32_t>::max();

		Vector<uint32_t> indices(indexCount, MaxUInt);
		Vector<FbxVertex> uniqueVertices;
		uniqueVertices.reserve(indexCount);

		for (int32_t i = 0; i < indexCount; i++)
		{
			if (i > 0 && AlmostEqual(vertices[i], vertices[i - 1]))
			{
				VT_ENSURE(indices[i - 1] < MaxUInt);
				indices[i] = indices[i - 1];
			}
			else
			{
				uniqueVertices.push_back(vertices[i]);
				indices[i] = static_cast<uint32_t>(uniqueVertices.size() - 1);
			}
		}

		VertexContainer vertexContainer{};
		vertexContainer.Resize(uniqueVertices.size());

		for (size_t i = 0; i < uniqueVertices.size(); i++)
		{
			vertexContainer.positions[i] = uniqueVertices[i].position;

			// Setup material data
			{
				auto& materialData = vertexContainer.materialData[i];

				const auto octNormal = Utility::OctNormalEncode(uniqueVertices[i].normal);

				materialData.normal.x = uint8_t(octNormal.x * 255u);
				materialData.normal.y = uint8_t(octNormal.y * 255u);
				materialData.tangent = Utility::EncodeTangent(uniqueVertices[i].normal, uniqueVertices[i].tangent);
				materialData.texCoords.x = static_cast<half_float::half>(uniqueVertices[i].texCoords.x);
				materialData.texCoords.y = static_cast<half_float::half>(uniqueVertices[i].texCoords.y);
			}
		}

		auto& subMesh = destinationMesh->m_subMeshes.emplace_back();
		subMesh.vertexStartOffset = static_cast<uint32_t>(destinationMesh->m_vertexContainer.positions.size());
		subMesh.indexStartOffset = static_cast<uint32_t>(destinationMesh->m_indices.size());
		subMesh.vertexCount = static_cast<uint32_t>(uniqueVertices.size());
		subMesh.indexCount = static_cast<uint32_t>(indices.size());
		subMesh.name = name;
		subMesh.materialIndex = static_cast<uint32_t>(uniqueVertices.front().material);

		destinationMesh->m_indices.append(indices);
		destinationMesh->m_vertexContainer.Append(vertexContainer, uniqueVertices.size());
	}

	void FbxSourceImporter::CreateVoltMeshFromFbxMesh(const fbxsdk::FbxMesh& fbxMesh, Ref<Mesh> destinationMesh, const Vector<Ref<Material>>& materials)
	{
		Vector<FbxVertex> vertices;
		CreateNonIndexedMesh(fbxMesh, vertices);
		TranslateNodeToSceneMaterials(fbxMesh.GetNode(), materials, vertices);

		// Sort vertices by material
		std::sort(vertices.begin(), vertices.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.material < rhs.material;
		});

		// Find sub meshes
		struct SubMeshRange
		{
			size_t first;
			size_t last;
		};

		Vector<SubMeshRange> subMeshRanges;

		size_t firstVertex = 0;
		for (size_t i = 1; i < vertices.size(); i++)
		{
			if (vertices[i].material != vertices[firstVertex].material)
			{
				subMeshRanges.emplace_back(firstVertex, i);
				firstVertex = i;
			}
		}
		subMeshRanges.emplace_back(firstVertex, vertices.size());

		// Create sub meshes
		for (const auto& [first, last] : subMeshRanges)
		{
			const size_t indexCount = last - first;
			CreateSubMeshFromVertexRange(destinationMesh, &vertices[first], indexCount, fbxMesh.GetName());
		}
	}

	Vector<Ref<Asset>> FbxSourceImporter::ImportAsStaticMesh(fbxsdk::FbxScene* fbxScene, const MeshSourceImportConfig& importConfig)
	{
		Vector<FbxMesh*> fbxMeshes;
		auto fetcher = [&](FbxMesh* mesh, FbxNode* node)
		{
			fbxMeshes.emplace_back(mesh);
		};

		VisitNodeAttributesOfType<FbxMesh>(fbxScene, fetcher);

		Vector<Ref<Material>> materials = CreateSceneMaterials(fbxScene, importConfig);
		Ref<Mesh> voltMesh = AssetManager::CreateAsset<Mesh>(importConfig.destinationDirectory, importConfig.destinationFilename);

		for (auto* fbxMesh : fbxMeshes)
		{
			CreateVoltMeshFromFbxMesh(*fbxMesh, voltMesh, materials);
		}

		for (uint32_t i = 0; i < static_cast<uint32_t>(materials.size()); i++)
		{
			voltMesh->m_materialTable.SetMaterial(materials[i]->handle, i);
		}

		voltMesh->Construct();

		Vector<Ref<Asset>> result;
		result.emplace_back(voltMesh);

		for (auto& material : materials)
		{
			result.emplace_back(material);
		}

		return result;
	}

	Vector<Ref<Asset>> FbxSourceImporter::ImportInternal(const std::filesystem::path& filepath, const void* config, const SourceAssetUserImportData& userData)
	{
		const MeshSourceImportConfig& importConfig = *reinterpret_cast<const MeshSourceImportConfig*>(config);

		FbxManagerPtr fbxManager(FbxManager::Create());
		if (!fbxManager)
		{
			VT_LOGC(Error, LogFbxSourceImporter, "Creating the FBX manager failed!");
			userData.OnError("The import process failed!");

			return {};
		}

		FbxIOSettingsPtr fbxIOSettings(FbxIOSettings::Create(fbxManager.get(), "FBX I/O settings"));
		if (!fbxIOSettings)
		{
			VT_LOGC(Error, LogFbxSourceImporter, "Creating FBX IO settings failed!");
			userData.OnError("The import process failed!");

			return {};
		}

		SetupIOSettings(*fbxIOSettings, importConfig, filepath);
		fbxManager->SetIOSettings(fbxIOSettings.get());

		FbxImporterPtr fbxImporter(fbxsdk::FbxImporter::Create(fbxManager.get(), "FBX importer"));
		if (!fbxImporter)
		{
			VT_LOGC(Error, LogFbxSourceImporter, "Creating FBX importer failed!");
			userData.OnError("The import process failed!");

			return {};
		}

		// Setup progress..

		if (!fbxImporter->Initialize(filepath.string().c_str(), -1, fbxIOSettings.get()))
		{
			if (fbxImporter->GetStatus() == FbxStatus::ePasswordError && !fbxIOSettings->GetBoolProp(IMP_FBX_PASSWORD_ENABLE, false))
			{
				const std::string password = userData.OnPasswordRrquired();

				if (!password.empty())
				{
					fbxIOSettings->SetStringProp(IMP_FBX_PASSWORD, password.c_str());
					fbxIOSettings->SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

					if (!fbxImporter->Initialize(filepath.string().c_str(), -1, fbxIOSettings.get()))
					{
						VT_LOGC(Error, LogFbxSourceImporter, "Initializing imported for password protected file failed!");
						userData.OnError("The import process failed!");

						return {};
					}
				}
				else
				{
					VT_LOGC(Error, LogFbxSourceImporter, "Initializing imported for password protected file failed, password required!");
					userData.OnError("The import process failed!");

					return {};
				}
			}

			const std::string message = "Cannot initialize FBX importer for " + filepath.string() + ": " + fbxImporter->GetStatus().GetErrorString();
			VT_LOGC(Error, LogFbxSourceImporter, message);
			userData.OnError(message);
			return {};
		}

		FbxScenePtr fbxScene(FbxScene::Create(fbxManager.get(), filepath.string().c_str()));
		if (!fbxScene)
		{
			VT_LOGC(Error, LogFbxSourceImporter, "Creating FBX scene failed!");
			userData.OnError("The import process failed!");

			return {};
		}

		if (!fbxImporter->Import(fbxScene.get()))
		{
			VT_LOGC(Error, LogFbxSourceImporter, "Importing FBX scene failed!");
			userData.OnError("The import process failed!");

			return {};
		}

		fbxManager->SetIOSettings(nullptr);
		fbxIOSettings.reset();
		fbxImporter.reset();

		PostProccessFbxScene(fbxScene.get(), importConfig, userData);

		Vector<Ref<Asset>> result;

		switch (importConfig.importType)
		{
			case MeshSourceImportType::StaticMesh:
			{
				result = ImportAsStaticMesh(fbxScene.get(), importConfig);
				break;
			}

			case MeshSourceImportType::SkeletalMesh:
			{
				break;
			}

			case MeshSourceImportType::Animation:
			{
				break;
			}
		}

		return result;
	}
}
