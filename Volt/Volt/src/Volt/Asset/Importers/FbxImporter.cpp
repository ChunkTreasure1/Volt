#include "vtpch.h"
#include "FbxImporter.h"

#include "Volt/Log/Log.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Asset/Rendering/MaterialTable.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/Animation/Animation.h"

#include "Volt/Rendering/Shader/ShaderMap.h"
#include "Volt/Rendering/Mesh/MeshCommon.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Math/Math.h"

#include <glm/glm.hpp>

#include <ufbx.h>

namespace Volt
{
	namespace Utility
	{
		inline glm::mat4 GetMatrix(const ufbx_matrix& matrix)
		{ 
			return
			{
				glm::vec4(matrix.m00, matrix.m10, matrix.m20, 0.0),
				glm::vec4(matrix.m01, matrix.m11, matrix.m21, 0.f),
				glm::vec4(matrix.m02, matrix.m12, matrix.m22, 0.f),
				glm::vec4(matrix.m03, matrix.m13, matrix.m23, 1.f),
			};
		}

		inline void PrintFBXError(const ufbx_error& error, std::string_view description)
		{
			char buffer[1024];
			ufbx_format_error(buffer, sizeof(buffer), &error);
			VT_CORE_ERROR("{0}: {1}", description, buffer);
		}
	}

	bool FbxImporter::ImportSkeletonImpl(const std::filesystem::path& path, Skeleton& dstSkeleton)
	{
		ufbx_scene* scene = LoadScene(path);
		if (!scene)
		{
			return false;
		}

		GatherSkeleton(dstSkeleton, scene->root_node, -1);

		if (dstSkeleton.m_joints.empty())
		{
			VT_CORE_ERROR("[FBXImporter]: Unable to import skeleton! There was no skeleton defined in FBX file!");

			ufbx_free_scene(scene);
			return false;
		}

		ReadSkinningData(dstSkeleton, scene);

		ufbx_free_scene(scene);
		return true;
	}

	bool FbxImporter::ImportAnimationImpl(const std::filesystem::path& path, Ref<Skeleton> targetSkeleton, Animation& dstAnimation)
	{
		ufbx_scene* scene = LoadScene(path);
		if (!scene)
		{
			return false;
		}

		for (const auto* stack : scene->anim_stacks)
		{
			ufbx_baked_anim* bakedAnim = ufbx_bake_anim(scene, stack->anim, nullptr, nullptr);
			if (!bakedAnim)
			{
				continue;
			}
		
			const double duration = stack->time_end - stack->time_begin;
			dstAnimation.m_duration = static_cast<float>(duration);

			// Find out frame count
			for (const auto& bakedNode : bakedAnim->nodes)
			{
				ufbx_node* sceneNode = scene->nodes[bakedNode.typed_id];

				std::string jointName = sceneNode->name.data;
				if (size_t namePos = jointName.find_last_of(":"); namePos != std::string::npos)
				{
					jointName = jointName.substr(namePos + 1);
				}

				const int32_t jntIndex = targetSkeleton->GetJointIndexFromName(jointName);
				if (jntIndex == -1)
				{
					continue;
				}

				if (dstAnimation.m_frames.size() < bakedNode.translation_keys.count)
				{
					dstAnimation.m_frames.resize(bakedNode.translation_keys.count);
				}

				for (size_t i = 0; const auto& keyFrame : bakedNode.translation_keys)
				{
					if (dstAnimation.m_frames.at(i).localTRS.size() < targetSkeleton->m_joints.size())
					{
						dstAnimation.m_frames.at(i).localTRS.resize(targetSkeleton->m_joints.size());
					}

					auto& trs = dstAnimation.m_frames.at(i).localTRS.at(jntIndex);
					trs.position = { keyFrame.value.x, keyFrame.value.y, keyFrame.value.z };
					i++;
				}

				for (size_t i = 0; const auto & keyFrame : bakedNode.rotation_keys)
				{
					if (dstAnimation.m_frames.at(i).localTRS.size() < targetSkeleton->m_joints.size())
					{
						dstAnimation.m_frames.at(i).localTRS.resize(targetSkeleton->m_joints.size());
					}

					auto& trs = dstAnimation.m_frames.at(i).localTRS.at(jntIndex);
					trs.rotation = glm::quat(static_cast<float>(keyFrame.value.w), static_cast<float>(keyFrame.value.x), static_cast<float>(keyFrame.value.y), static_cast<float>(keyFrame.value.z));
					i++;
				}

				for (size_t i = 0; const auto & keyFrame : bakedNode.scale_keys)
				{
					if (dstAnimation.m_frames.at(i).localTRS.size() < targetSkeleton->m_joints.size())
					{
						dstAnimation.m_frames.at(i).localTRS.resize(targetSkeleton->m_joints.size());
					}

					auto& trs = dstAnimation.m_frames.at(i).localTRS.at(jntIndex);
					trs.scale = { keyFrame.value.x, keyFrame.value.y, keyFrame.value.z };
					i++;
				}
			}

			ufbx_free_baked_anim(bakedAnim);
		}

		ufbx_free_scene(scene);
		return true;
	}

	void FbxImporter::ExportMeshImpl(std::vector<Ref<Mesh>>, const std::filesystem::path&)
	{
		
	}

	ufbx_scene* FbxImporter::LoadScene(const std::filesystem::path& path)
	{
		ufbx_load_opts options =
		{ 
			.load_external_files = true,
			.ignore_missing_external_files = true,
			.generate_missing_normals = true,

			.target_axes =
			{
				.right = UFBX_COORDINATE_AXIS_POSITIVE_X,
				.up = UFBX_COORDINATE_AXIS_POSITIVE_Y,
				.front = UFBX_COORDINATE_AXIS_POSITIVE_Z,
			},
			.target_unit_meters = 1.f, 
		};

		ufbx_error error;
		ufbx_scene* scene = ufbx_load_file(path.string().c_str(), &options, &error);
		if (!scene)
		{
			Utility::PrintFBXError(error, "Failed to load scene");
			return nullptr;
		}

		return scene;
	}

	void FbxImporter::ReadMesh(Mesh& dstMesh, ufbx_mesh* mesh)
	{
		// Create materials
		for (size_t i = 0; i < mesh->materials.count; i++)
		{
			ufbx_material* material = mesh->materials.data[i];
			
			auto newMaterial = AssetManager::CreateAsset<Material>("", material->name.data);
			dstMesh.m_materialTable.SetMaterial(newMaterial->handle, material->typed_id);
		}

		if (mesh->materials.count == 0)
		{
			auto newMaterial = AssetManager::CreateAsset<Material>("", "None");
			dstMesh.m_materialTable.SetMaterial(newMaterial->handle, 0);
		}

		// Read sub meshes
		size_t maxParts = 0;
		size_t maxTriangles = 0;

		for (size_t i = 0; i < mesh->material_parts.count; i++)
		{
			ufbx_mesh_part* part = &mesh->material_parts.data[i];
			if (part->num_triangles == 0)
			{
				continue;
			}

			maxParts++;
			maxTriangles = std::max(maxTriangles, part->num_triangles);
		}

		size_t maxIndexCount = mesh->max_face_triangles * 3;
		std::vector<uint32_t> triangleIndices(maxIndexCount);
		std::vector<uint32_t> indices(maxTriangles * 3);
		std::vector<Vertex> vertices(maxTriangles * 3);

		std::vector<glm::vec3> vertexPositions(maxTriangles * 3);
		std::vector<VertexMaterialData> vertexMaterialData(maxTriangles * 3);
		std::vector<VertexAnimationInfo> vertexAnimationInfo(maxTriangles * 3);

		std::vector<uint16_t> vertexBoneInfluences;
		std::vector<float> vertexBoneWeights;

		if (mesh->skin_deformers.count > 0)
		{
			ufbx_skin_deformer* skin = mesh->skin_deformers.data[0];

			for (size_t i = 0; i < mesh->num_vertices; i++)
			{
				size_t numWeights = 0;
				float totalWeight = 0.f;
				std::vector<float> weights{};
				std::vector<uint16_t> influences{};

				ufbx_skin_vertex vertexWeights = skin->vertices.data[i];
				for (size_t j = 0; j < vertexWeights.num_weights; j++)
				{
					ufbx_skin_weight weight = skin->weights.data[vertexWeights.weight_begin + j];
					totalWeight += static_cast<float>(weight.weight);
					influences.push_back(static_cast<uint16_t>(weight.cluster_index));
					weights.push_back(static_cast<float>(weight.weight));

					numWeights++;
				}

				vertexAnimationInfo[i].boneOffset = static_cast<uint16_t>(vertexBoneInfluences.size() + dstMesh.m_vertexBoneInfluences.size());
				vertexBoneInfluences.resize(vertexBoneInfluences.size() + numWeights);
				vertexBoneWeights.resize(vertexBoneInfluences.size() + numWeights);

				if (totalWeight > 0.f)
				{
					vertexAnimationInfo[i].influenceCount = static_cast<uint16_t>(numWeights);

					for (size_t j = 0; j < weights.size(); j++)
					{
						vertexBoneInfluences[j] = influences[j];
						vertexBoneWeights[j] = weights.at(j);
					}
				}
			}
		}

		for (size_t i = 0; i < mesh->material_parts.count; i++)
		{
			ufbx_mesh_part& meshPart = mesh->material_parts.data[i];
			if (meshPart.num_triangles == 0)
			{
				continue;
			}

			size_t numIndices = 0;
			for (size_t j = 0; j < meshPart.num_faces; j++)
			{
				ufbx_face face = mesh->faces.data[meshPart.face_indices.data[j]];
				size_t numTris = ufbx_triangulate_face(triangleIndices.data(), maxIndexCount, mesh, face);

				ufbx_vec2 defaultUV = { 0 };
				ufbx_vec3 defaultTangent = { 0, 1, 0 };

				for (size_t k = 0; k < numTris * 3; k++)
				{
					uint32_t index = triangleIndices[k];
					Vertex& vertex = vertices[numIndices];

					ufbx_vec3 pos = ufbx_get_vertex_vec3(&mesh->vertex_position, index);
					ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, index);
					ufbx_vec3 tangent = mesh->vertex_tangent.exists ? ufbx_get_vertex_vec3(&mesh->vertex_tangent, index) : defaultTangent;
					ufbx_vec2 uv = mesh->vertex_uv.exists ? ufbx_get_vertex_vec2(&mesh->vertex_uv, index) : defaultUV;

					vertexPositions[numIndices] = { pos.x, pos.y, pos.z };
					
					{
						auto& materialData = vertexMaterialData[numIndices];
						
						const glm::vec3 vNormal = { normal.x, normal.y, normal.z };
						const glm::vec3 vTangent = { tangent.x, tangent.y, tangent.z };

						const auto octNormal = Utility::OctNormalEncode(vNormal);

						materialData.normal.x = uint8_t(octNormal.x * 255u);
						materialData.normal.y = uint8_t(octNormal.y * 255u);
						materialData.tangent = Utility::EncodeTangent(vNormal, vTangent);
						materialData.texCoords.x = static_cast<half_float::half>(static_cast<float>(uv.x));
						materialData.texCoords.y = static_cast<half_float::half>(static_cast<float>(uv.y));
					}
					
					vertex.position = { pos.x, pos.y, pos.z };
					vertex.normal = { normal.x, normal.y, normal.z };
					vertex.tangent = { tangent.x, tangent.y, tangent.z };
					vertex.uv = { uv.x, uv.y };

					numIndices++;
				}
			}

			ufbx_vertex_stream streams[3];
			streams[0].data = vertexPositions.data();
			streams[0].vertex_count = numIndices;
			streams[0].vertex_size = sizeof(glm::vec3);

			streams[1].data = vertexMaterialData.data();
			streams[1].vertex_count = numIndices;
			streams[1].vertex_size = sizeof(VertexMaterialData);
			
			streams[2].data = vertexAnimationInfo.data();
			streams[2].vertex_count = numIndices;
			streams[2].vertex_size = sizeof(VertexAnimationInfo);

			ufbx_error error;
			size_t numVertices = ufbx_generate_indices(streams, 3, indices.data(), numIndices, nullptr, &error);
			if (error.type != UFBX_ERROR_NONE)
			{
				Utility::PrintFBXError(error, "Failed to generate index buffer");
				continue;
			}

			if (!mesh->vertex_tangent.exists)
			{
				VT_CORE_ERROR("[FbxImporter]: No tangents availiable in mesh, shading might be incorrect!");
			}

			auto& subMesh = dstMesh.m_subMeshes.emplace_back();
			subMesh.vertexStartOffset = static_cast<uint32_t>(dstMesh.m_vertexContainer.positions.size());
			subMesh.indexStartOffset = static_cast<uint32_t>(dstMesh.m_indices.size());
			subMesh.vertexCount = static_cast<uint32_t>(numVertices);
			subMesh.indexCount = static_cast<uint32_t>(numIndices);
			subMesh.name = mesh->name.data;
			
			if (meshPart.index < mesh->materials.count)
			{
				ufbx_material* material = mesh->materials.data[meshPart.index];
				subMesh.materialIndex = material->typed_id;
			}
			else
			{
				subMesh.materialIndex = 0;
			}

			dstMesh.m_indices.insert(dstMesh.m_indices.end(), indices.begin(), indices.begin() + numIndices);

			dstMesh.m_vertexContainer.positions.insert(dstMesh.m_vertexContainer.positions.end(), vertexPositions.begin(), vertexPositions.begin() + numVertices);
			dstMesh.m_vertexContainer.materialData.insert(dstMesh.m_vertexContainer.materialData.end(), vertexMaterialData.begin(), vertexMaterialData.begin() + numVertices);
			dstMesh.m_vertexContainer.vertexAnimationInfo.insert(dstMesh.m_vertexContainer.vertexAnimationInfo.end(), vertexAnimationInfo.begin(), vertexAnimationInfo.begin() + numVertices);
			dstMesh.m_vertexContainer.vertexBoneInfluences.insert(dstMesh.m_vertexContainer.vertexBoneInfluences.end(), vertexBoneInfluences.begin(), vertexBoneInfluences.end());
			dstMesh.m_vertexContainer.vertexBoneWeights.insert(dstMesh.m_vertexContainer.vertexBoneWeights.end(), vertexBoneWeights.begin(), vertexBoneWeights.end());
		}
	}

	void FbxImporter::GatherSkeleton(Skeleton& skeleton, const ufbx_node* currentNode, int32_t parentIndex)
	{
		int32_t currentIndex = parentIndex;

		if (currentNode->bone)
		{
			currentIndex = static_cast<int32_t>(skeleton.m_joints.size());

			auto& joint = skeleton.m_joints.emplace_back();

			std::string jointName = currentNode->name.data;
			if (size_t namePos = jointName.find_last_of(":"); namePos != std::string::npos)
			{
				jointName = jointName.substr(namePos + 1);
			}

			joint.name = jointName;
			joint.parentIndex = parentIndex;

			skeleton.m_jointNameToIndex[joint.name] = currentIndex;
		}
		
		for (const auto* childNode : currentNode->children)
		{
			GatherSkeleton(skeleton, childNode, currentIndex);
		}
	}

	void FbxImporter::ReadSkinningData(Skeleton& skeleton, const ufbx_scene* scene)
	{
		skeleton.m_inverseBindPose.resize(skeleton.m_joints.size());
		skeleton.m_restPose.resize(skeleton.m_joints.size());
		std::vector<glm::mat4> bindPoses(skeleton.m_joints.size(), glm::identity<glm::mat4>());

		for (const auto* skin : scene->skin_deformers)
		{
			for (const auto* cluster : skin->clusters)
			{
				std::string jointName = cluster->bone_node->name.data;
				if (size_t namePos = jointName.find_last_of(":"); namePos != std::string::npos)
				{
					jointName = jointName.substr(namePos + 1);
				}

				size_t jointIndex = skeleton.m_jointNameToIndex[jointName];
				bindPoses[jointIndex] = glm::transpose(Utility::GetMatrix(cluster->bind_to_world));

				skeleton.m_inverseBindPose[jointIndex] = Utility::GetMatrix(cluster->geometry_to_bone);
			}
		}

		for (size_t i = 0; i < skeleton.m_joints.size(); i++)
		{
			glm::mat4 parentTransform = glm::identity<glm::mat4>();

			auto& joint = skeleton.m_joints[i];
			if (joint.parentIndex >= 0)
			{
				parentTransform = bindPoses[joint.parentIndex];
			}

			const glm::mat4 localPose = glm::inverse(parentTransform) * bindPoses[i];

			glm::vec3 translation;
			glm::vec3 scale;
			glm::quat rotation;
			::Math::Decompose(localPose, translation, rotation, scale);
		
			skeleton.m_restPose[i].position = translation;
			skeleton.m_restPose[i].rotation = rotation;
			skeleton.m_restPose[i].scale = scale;
		}
	}

	bool FbxImporter::ImportMeshImpl(const std::filesystem::path& path, Mesh& destMesh)
	{
		ufbx_scene* scene = LoadScene(path);
		if (!scene)
		{
			return false;
		}

		for (size_t i = 0; i < scene->meshes.count; i++)
		{
			ReadMesh(destMesh, scene->meshes.data[i]);
		}

		ufbx_free_scene(scene);

		destMesh.Construct();
		return true;
	}
}
