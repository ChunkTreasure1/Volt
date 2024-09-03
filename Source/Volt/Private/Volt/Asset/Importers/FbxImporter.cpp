#include "vtpch.h"
#include "Volt/Asset/Importers/FbxImporter.h"

#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Asset/Rendering/MaterialTable.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/Animation/Animation.h"

#include "Volt/Rendering/Mesh/MeshCommon.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Math/Math.h"

#include "Volt/Math/Math.h"

#include <AssetSystem/AssetManager.h>
#include <RenderCore/Shader/ShaderMap.h>

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

		inline glm::vec3 GetVec(const ufbx_vec3& vec)
		{
			return { static_cast<float>(vec.x), static_cast<float>(vec.y), static_cast<float>(vec.z) };
		}

		inline glm::quat GetQuat(const ufbx_quat& quat)
		{
			return { static_cast<float>(quat.w), static_cast<float>(quat.x), static_cast<float>(quat.y), static_cast<float>(quat.z) };
		}

		inline void PrintFBXError(const ufbx_error& error, std::string_view description)
		{
			char buffer[1024];
			ufbx_format_error(buffer, sizeof(buffer), &error);
			VT_LOG(Error, "{0}: {1}", description, buffer);
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
			VT_LOG(Error, "[FBXImporter]: Unable to import skeleton! There was no skeleton defined in FBX file!");

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

		if (scene->anim_stacks.count > 0)
		{
			const auto* stack = scene->anim_stacks[0];

			const double frameRate = scene->settings.frames_per_second;
			float duration = (float)stack->time_end - (float)stack->time_begin;
			size_t numFrames = static_cast<size_t>(duration * frameRate) + 1;

			dstAnimation.m_duration = static_cast<float>(duration);
			dstAnimation.m_framesPerSecond = static_cast<uint32_t>(frameRate);

			dstAnimation.m_frames.resize(numFrames);
			for (auto& frame : dstAnimation.m_frames)
			{
				frame.localTRS.resize(targetSkeleton->GetJointCount());
			}

			for (const auto* node : scene->nodes)
			{
				std::string jointName = node->name.data;
				if (size_t namePos = jointName.find_last_of(":"); namePos != std::string::npos)
				{
					jointName = jointName.substr(namePos + 1);
				}

				const int32_t jntIndex = targetSkeleton->GetJointIndexFromName(jointName);
				if (jntIndex == -1)
				{
					continue;
				}

				for (size_t i = 0; i < numFrames; i++)
				{
					double time = stack->time_begin + static_cast<double>(i) / frameRate;
					ufbx_transform transform = ufbx_evaluate_transform(stack->anim, node, time);

					auto& trs = dstAnimation.m_frames.at(i).localTRS.at(jntIndex);
					trs.position = Utility::GetVec(transform.translation);
					trs.position.x = -trs.position.x;

					trs.rotation = Utility::GetQuat(transform.rotation);
					trs.rotation.y = -trs.rotation.y;
					trs.rotation.z = -trs.rotation.z;

					trs.scale = Utility::GetVec(transform.scale);

					if (i > 0)
					{
						if (glm::dot(trs.rotation, dstAnimation.m_frames.at(i).localTRS.at(jntIndex).rotation) < 0.f)
						{
							trs.rotation = { -trs.rotation.w, -trs.rotation.x, -trs.rotation.y, -trs.rotation.z };
						}
					}
				}
			}
		}


		ufbx_free_scene(scene);
		return true;
	}

	void FbxImporter::ExportMeshImpl(Vector<Ref<Mesh>>, const std::filesystem::path&)
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
			.target_unit_meters = 0.01f,
		};

		options.space_conversion = UFBX_SPACE_CONVERSION_MODIFY_GEOMETRY;

		ufbx_error error;
		ufbx_scene* scene = ufbx_load_file(path.string().c_str(), &options, &error);

		if (!scene)
		{
			Utility::PrintFBXError(error, "Failed to load scene");
			return nullptr;
		}

		return scene;
	}

	void FbxImporter::ReadMesh(Mesh& dstMesh, Skeleton& skeleton, ufbx_mesh* mesh)
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
		Vector<uint32_t> triangleIndices(maxIndexCount);
		Vector<uint32_t> indices(maxTriangles * 3);
		
		VertexContainer vertexContainer{};
		vertexContainer.Resize(maxTriangles * 3);

		Vector<uint16_t> vertexBoneInfluences;
		Vector<float> vertexBoneWeights;

		ufbx_skin_deformer* skin = nullptr;

		if (mesh->skin_deformers.count > 0)
		{
			skin = mesh->skin_deformers.data[0];
		}

		for (size_t i = 0; i < mesh->material_parts.count; i++)
		{
			ufbx_mesh_part& meshPart = mesh->material_parts.data[i];
			if (meshPart.num_triangles == 0)
			{
				continue;
			}

			uint32_t numIndices = 0;
			for (size_t j = 0; j < meshPart.num_faces; j++)
			{
				ufbx_face face = mesh->faces.data[meshPart.face_indices.data[j]];
				size_t numTris = ufbx_triangulate_face(triangleIndices.data(), maxIndexCount, mesh, face);

				ufbx_vec2 defaultUV = { 0 };
				ufbx_vec3 defaultTangent = { 0, 1, 0 };

				for (size_t k = 0; k < numTris * 3; k++)
				{
					const uint32_t index = triangleIndices[k];
					const uint32_t vertexIndex = mesh->vertex_indices[index];

					ufbx_vec3 pos = ufbx_get_vertex_vec3(&mesh->vertex_position, index);
					ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, index);
					ufbx_vec3 tangent = mesh->vertex_tangent.exists ? ufbx_get_vertex_vec3(&mesh->vertex_tangent, index) : defaultTangent;
					ufbx_vec2 uv = mesh->vertex_uv.exists ? ufbx_get_vertex_vec2(&mesh->vertex_uv, index) : defaultUV;

					vertexContainer.positions[numIndices] = { pos.x, pos.y, pos.z };

					{
						auto& materialData = vertexContainer.materialData[numIndices];

						const glm::vec3 vNormal = { normal.x, normal.y, normal.z };
						const glm::vec3 vTangent = { tangent.x, tangent.y, tangent.z };

						const auto octNormal = Utility::OctNormalEncode(vNormal);

						materialData.normal.x = uint8_t(octNormal.x * 255u);
						materialData.normal.y = uint8_t(octNormal.y * 255u);
						materialData.tangent = Utility::EncodeTangent(vNormal, vTangent);
						materialData.texCoords.x = static_cast<half_float::half>(static_cast<float>(uv.x));
						materialData.texCoords.y = static_cast<half_float::half>(static_cast<float>(uv.y));
					}

					if (skin && skeleton.m_joints.size() > 0)
					{
						auto& animData = vertexContainer.animationData[numIndices];

						ufbx_skin_vertex skinVertex = skin->vertices[vertexIndex];
						const uint32_t numWeights = std::min(skinVertex.num_weights, 4u);

						float totalWeight = 0.f;
						for (uint32_t w = 0; w < numWeights; w++)
						{
							ufbx_skin_weight skinWeight = skin->weights[skinVertex.weight_begin + w];
							std::string jointName = skin->clusters[skinWeight.cluster_index]->bone_node->name.data;
							if (size_t namePos = jointName.find_last_of(":"); namePos != std::string::npos)
							{
								jointName = jointName.substr(namePos + 1);
							}

							animData.influences[(int)w] = static_cast<uint32_t>(skeleton.m_jointNameToIndex.at(jointName));
							animData.weights[(int)w] = static_cast<float>(skinWeight.weight);

							totalWeight += static_cast<float>(skinWeight.weight);
						}

						if (totalWeight > 0.f)
						{
							for (uint32_t w = 0; w < numWeights; w++)
							{
								animData.weights[w] /= totalWeight;
							}
						}
					}

					numIndices++;
				}
			}

			indices.resize(meshPart.num_triangles * 3);

			Vector<ufbx_vertex_stream> streams{};

			{
				auto& stream = streams.emplace_back();
				stream.data = vertexContainer.positions.data();
				stream.vertex_count = numIndices;
				stream.vertex_size = sizeof(glm::vec3);
			}

			{
				auto& stream = streams.emplace_back();
				stream.data = vertexContainer.materialData.data();
				stream.vertex_count = numIndices;
				stream.vertex_size = sizeof(VertexMaterialData);
			}
			
			{
				auto& stream = streams.emplace_back();
				stream.data = vertexContainer.animationInfo.data();
				stream.vertex_count = numIndices;
				stream.vertex_size = sizeof(VertexAnimationInfo);
			}

			{
				auto& stream = streams.emplace_back();
				stream.data = vertexContainer.animationData.data();
				stream.vertex_count = numIndices;
				stream.vertex_size = sizeof(VertexAnimationData);
			}
			
			ufbx_error error;
			size_t numVertices = ufbx_generate_indices(streams.data(), streams.size(), indices.data(), indices.size(), nullptr, &error);
			if (error.type != UFBX_ERROR_NONE)
			{
				Utility::PrintFBXError(error, "Failed to generate index buffer");
				continue;
			}

			if (!mesh->vertex_tangent.exists)
			{
				VT_LOG(Error, "[FbxImporter]: No tangents availiable in mesh, shading might be incorrect!");
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
			dstMesh.m_vertexContainer.Append(vertexContainer, numVertices);
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
		skeleton.m_inverseBindPose.resize(skeleton.m_joints.size(), glm::identity<glm::mat4>());
		skeleton.m_restPose.resize(skeleton.m_joints.size());
		Vector<glm::mat4> bindPoses(skeleton.m_joints.size(), glm::identity<glm::mat4>());

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

				glm::vec3 translation;
				glm::vec3 scale;
				glm::quat rotation;
				::Math::Decompose(Utility::GetMatrix(cluster->geometry_to_bone), translation, rotation, scale);

				translation.x = -translation.x;

				rotation.y = -rotation.y;
				rotation.z = -rotation.z;

				skeleton.m_inverseBindPose[jointIndex] = glm::translate(glm::mat4{ 1.f }, translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4{ 1.f }, scale);
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

		Skeleton skeleton;
		GatherSkeleton(skeleton, scene->root_node, -1);

		for (size_t i = 0; i < scene->meshes.count; i++)
		{
			ReadMesh(destMesh, skeleton, scene->meshes.data[i]);
		}

		ufbx_free_scene(scene);

		destMesh.Construct();
		return true;
	}
}
