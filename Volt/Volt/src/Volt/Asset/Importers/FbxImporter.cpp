#include "vtpch.h"
#include "FbxImporter.h"

#include "Volt/Log/Log.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Asset/Rendering/MaterialTable.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/Animation/Animation.h"

#include "Volt/RenderingNew/Shader/ShaderMap.h"

#include "Volt/Project/ProjectManager.h"

#include <glm/glm.hpp>

namespace Volt
{
	void FbxImporter::ImportMeshImpl(const std::filesystem::path& path, Ref<Mesh>& mesh)
	{
		TGA::FBX::Importer::InitImporter();

		TGA::FBX::Mesh tgaMesh;
		try
		{
			if (!TGA::FBX::Importer::LoadMeshW(path.wstring(), tgaMesh))
			{
				mesh->SetFlag(AssetFlag::Invalid, true);
				return;
			}
		}
		catch (const std::exception& e)
		{
			VT_CORE_ERROR("[FBXImporter] Unable to import animation! Reason: {0}", e.what());
			mesh->SetFlag(AssetFlag::Invalid, true);
			return;
		}

		if (!tgaMesh.IsValid())
		{
			mesh->SetFlag(AssetFlag::Invalid, true);
			return;
		}

		for (const auto& element : tgaMesh.Elements)
		{
			auto& newSubMesh = mesh->m_subMeshes.emplace_back();
			newSubMesh.vertexStartOffset = static_cast<uint32_t>(mesh->m_vertices.size());
			newSubMesh.indexStartOffset = static_cast<uint32_t>(mesh->m_indices.size());
			newSubMesh.vertexCount = static_cast<uint32_t>(element.Vertices.size());
			newSubMesh.indexCount = static_cast<uint32_t>(element.Indices.size());
			newSubMesh.materialIndex = element.MaterialIndex;
			newSubMesh.name = element.MeshName;

			const glm::vec3 translation = { element.localTranslation[0], element.localTranslation[1], element.localTranslation[2] };
			const glm::vec3 rotation = { element.localRotation[0], element.localRotation[1], element.localRotation[2] };
			const glm::vec3 scale = { element.localScale[0], element.localScale[1], element.localScale[2] };

			const glm::mat4 localTransform = glm::translate(glm::mat4(1.f), translation) *
				glm::mat4_cast(glm::quat(glm::radians(rotation))) * glm::scale(glm::mat4(1.f), scale);

			newSubMesh.transform = localTransform;

			for (const auto& tgaVertex : element.Vertices)
			{
				auto& newVertex = mesh->m_vertices.emplace_back();
				newVertex.position = *reinterpret_cast<const glm::vec4*>(tgaVertex.Position);
				newVertex.normal = *reinterpret_cast<const glm::vec3*>(tgaVertex.Normal);
				newVertex.tangent = *reinterpret_cast<const glm::vec3*>(tgaVertex.Tangent);
				newVertex.uv = *reinterpret_cast<const glm::vec2*>(tgaVertex.UVs[0]);
				newVertex.influences = *reinterpret_cast<const glm::uvec4*>(tgaVertex.BoneIDs);
				newVertex.weights = *reinterpret_cast<const glm::vec4*>(tgaVertex.BoneWeights);
			}

			mesh->m_indices.insert(mesh->m_indices.end(), element.Indices.begin(), element.Indices.end());
		}

		if (tgaMesh.Materials.empty())
		{
			auto newMaterial = AssetManager::CreateAsset<Material>("", "None");
			mesh->m_materialTable.SetMaterial(newMaterial->handle, 0);
		}
		else
		{
			for (uint32_t index = 0; const auto& material : tgaMesh.Materials)
			{
				auto newMaterial = AssetManager::CreateAsset<Material>("", material.MaterialName);
				mesh->m_materialTable.SetMaterial(newMaterial->handle, index);
				index++;
			}
		}

		mesh->m_boundingBox = BoundingBox{ *reinterpret_cast<const glm::vec3*>(tgaMesh.BoxBounds.Max), *reinterpret_cast<const glm::vec3*>(tgaMesh.BoxBounds.Min) };
		mesh->m_boundingSphere.center = { tgaMesh.BoxSphereBounds.Center[0], tgaMesh.BoxSphereBounds.Center[1], tgaMesh.BoxSphereBounds.Center[2] };
		mesh->m_boundingSphere.radius = tgaMesh.BoxSphereBounds.Radius;

		TGA::FBX::Importer::UninitImporter();

		mesh->Construct();
	}

	void FbxImporter::ImportSkeletonImpl(const std::filesystem::path& path, Ref<Skeleton>& skeleton)
	{
		TGA::FBX::Importer::InitImporter();

		TGA::FBX::Mesh tgaMesh;

		try
		{
			if (!TGA::FBX::Importer::LoadMeshW(path.wstring(), tgaMesh))
			{
				skeleton->SetFlag(AssetFlag::Invalid, true);
				return;
			}
		}
		catch (const std::exception& e)
		{
			VT_CORE_ERROR("[FBXImporter] Unable to import animation! Reason: {0}", e.what());
			skeleton->SetFlag(AssetFlag::Invalid, true);
			return;
		}

		skeleton->myJoints.resize(tgaMesh.Skeleton.Bones.size());
		skeleton->myInverseBindPose.resize(tgaMesh.Skeleton.Bones.size());
		skeleton->myRestPose.resize(tgaMesh.Skeleton.Bones.size());

		ProcessSkeleton(skeleton, tgaMesh.Skeleton.Bones, 0);

		TGA::FBX::Importer::UninitImporter();
	}

	void FbxImporter::ImportAnimationImpl(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Ref<Animation>& animation)
	{
		TGA::FBX::Importer::InitImporter();

		TGA::FBX::Animation tgaAnimation;

		try
		{
			if (!TGA::FBX::Importer::LoadAnimationW(path.wstring(), tgaAnimation))
			{
				animation->SetFlag(AssetFlag::Invalid, true);
				return;
			}
		}
		catch (const std::exception& e)
		{
			VT_CORE_ERROR("[FBXImporter] Unable to import animation! Reason: {0}", e.what());
			animation->SetFlag(AssetFlag::Invalid, true);
			return;
		}

		animation->myFramesPerSecond = static_cast<uint32_t>(tgaAnimation.FramesPerSecond);
		animation->myDuration = static_cast<float>(tgaAnimation.Duration);

		for (const auto& tgaFrame : tgaAnimation.Frames)
		{
			auto& newFrame = animation->myFrames.emplace_back();
			newFrame.localTRS.resize(targetSkeleton->myJoints.size());

			for (const auto& [jointName, localTQS] : tgaFrame.LocalTQS)
			{
				Animation::TRS animTRS{};
				animTRS.position = { localTQS.translation[0], localTQS.translation[1], localTQS.translation[2] };
				animTRS.rotation = { localTQS.rotation[3], localTQS.rotation[0], localTQS.rotation[1], localTQS.rotation[2] };
				animTRS.scale = { localTQS.scale[0], localTQS.scale[1] , localTQS.scale[2] };

				const int32_t jntIndex = targetSkeleton->GetJointIndexFromName(jointName);
				if (jntIndex == -1)
				{
					continue;
				}

				newFrame.localTRS[jntIndex] = animTRS;
			}
		}

		TGA::FBX::Importer::UninitImporter();
	}

	void FbxImporter::ExportMeshImpl(std::vector<Ref<Mesh>>, const std::filesystem::path&)
	{
		//FbxManager* sdkManager = FbxManager::Create();
		//FbxIOSettings* ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT);
		//sdkManager->SetIOSettings(ioSettings);

		//// Setup IOSettings

		//FbxExporter* exporter = FbxExporter::Create(sdkManager, "");

		//bool exportStatus = exporter->Initialize((Volt::ProjectManager::GetDirectory() / path).string().c_str(), -1, sdkManager->GetIOSettings());
		//if (!exportStatus)
		//{
		//	VT_CORE_ERROR("Unable to export to FBX file: {0}!", exporter->GetStatus().GetErrorString());
		//	return;
		//}

		//FbxScene* scene = FbxScene::Create(sdkManager, "Exported Scene");
		//FbxNode* rootNode = scene->GetRootNode();

		//for (const auto& meshAsset : assets)
		//{
		//	FbxNode* assetNode = FbxNode::Create(scene, meshAsset->path.stem().string().c_str());
		//	const auto& vertices = meshAsset->GetVertices();
		//	const auto& indices = meshAsset->GetIndices();

		//	for (const auto& subMesh : meshAsset->GetSubMeshes())
		//	{
		//		FbxNode* meshNode = FbxNode::Create(scene, subMesh.name.empty() ? "Mesh" : subMesh.name.c_str());
		//		FbxMesh* mesh = FbxMesh::Create(scene, "mesh");

		//		meshNode->SetNodeAttribute(mesh);
		//		mesh->InitControlPoints((int32_t)subMesh.vertexCount);

		//		fbxsdk::FbxLayer* layer = mesh->GetLayer(0);
		//		if (!layer)
		//		{
		//			mesh->CreateLayer();
		//			layer = mesh->GetLayer(0);
		//		}

		//		// Setup normal layers
		//		fbxsdk::FbxLayerElementNormal* layerElementNormal = fbxsdk::FbxLayerElementNormal::Create(mesh, "");
		//		layerElementNormal->SetMappingMode(fbxsdk::FbxLayerElement::eByControlPoint);
		//		layerElementNormal->SetReferenceMode(fbxsdk::FbxLayerElement::eIndexToDirect);

		//		// Setup tangent layers
		//		fbxsdk::FbxLayerElementTangent* layerElementTangent = fbxsdk::FbxLayerElementTangent::Create(mesh, "");
		//		layerElementTangent->SetMappingMode(fbxsdk::FbxLayerElement::eByControlPoint);
		//		layerElementTangent->SetReferenceMode(fbxsdk::FbxLayerElement::eIndexToDirect);

		//		// Setup binormal layers
		//		fbxsdk::FbxLayerElementBinormal* layerElementBinormal = fbxsdk::FbxLayerElementBinormal::Create(mesh, "");
		//		layerElementBinormal->SetMappingMode(fbxsdk::FbxLayerElement::eByControlPoint);
		//		layerElementBinormal->SetReferenceMode(fbxsdk::FbxLayerElement::eIndexToDirect);

		//		auto* controlPoints = mesh->GetControlPoints();

		//		for (uint32_t counter = 0, i = subMesh.vertexStartOffset; i < subMesh.vertexStartOffset + subMesh.vertexCount; i++, counter++)
		//		{
		//			controlPoints[counter] = Utility::Vec3ToFbxVector4(vertices.at(i).position);
		//			layerElementNormal->GetDirectArray().Add(Utility::Vec3ToFbxVector4(vertices.at(i).normal));
		//			layerElementTangent->GetDirectArray().Add(Utility::Vec3ToFbxVector4(vertices.at(i).tangent));
		//		}

		//		for (uint32_t i = subMesh.indexStartOffset; i < subMesh.indexStartOffset + subMesh.indexCount; i++)
		//		{
		//			layerElementNormal->GetIndexArray().Add((int32_t)indices.at(i));
		//			layerElementTangent->GetIndexArray().Add((int32_t)indices.at(i));
		//			layerElementBinormal->GetIndexArray().Add((int32_t)indices.at(i));
		//		}

		//		layer->SetNormals(layerElementNormal);
		//		layer->SetTangents(layerElementTangent);
		//		layer->SetBinormals(layerElementBinormal);

		//		assetNode->AddChild(meshNode);
		//	}

		//	rootNode->AddChild(assetNode);
		//}

		//if (!exporter->Export(scene))
		//{
		//	VT_CORE_ERROR("Unable to export to FBX file: {0}!", exporter->GetStatus().GetErrorString());
		//}
		//exporter->Destroy();
	}

	void FbxImporter::ProcessSkeleton(Ref<Skeleton> skeleton, const std::vector<TGA::FBX::Skeleton::Bone>& bones, uint32_t currentIndex)
	{
		const auto& currentJoint = bones.at(currentIndex);

		skeleton->myJoints[currentIndex].name = currentJoint.Name;
		skeleton->myJoints[currentIndex].parentIndex = currentJoint.ParentIdx;
		skeleton->myInverseBindPose[currentIndex] = glm::transpose(*reinterpret_cast<const glm::mat4*>(currentJoint.BindPoseInverse.Data));
		skeleton->myJointNameToIndex[currentJoint.Name] = static_cast<size_t>(currentIndex);

		skeleton->myRestPose[currentIndex].position = { currentJoint.restPosition[0], currentJoint.restPosition[1], currentJoint.restPosition[2] };

		skeleton->myRestPose[currentIndex].rotation.x = currentJoint.restRotation[0];
		skeleton->myRestPose[currentIndex].rotation.y = currentJoint.restRotation[1];
		skeleton->myRestPose[currentIndex].rotation.z = currentJoint.restRotation[2];
		skeleton->myRestPose[currentIndex].rotation.w = currentJoint.restRotation[3];
		
		skeleton->myRestPose[currentIndex].scale = { currentJoint.restScale[0], currentJoint.restScale[1], currentJoint.restScale[2] };

		for (const auto& child : currentJoint.Children)
		{
			ProcessSkeleton(skeleton, bones, child);
		}
	}
}
