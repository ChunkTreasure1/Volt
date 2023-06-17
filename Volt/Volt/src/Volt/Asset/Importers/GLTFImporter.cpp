#include "vtpch.h"
#include "GLTFImporter.h"

#include "Volt/Log/Log.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Rendering/RenderPipeline/ShaderRegistry.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE 
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

namespace Volt
{
	Ref<Mesh> GLTFImporter::ImportMeshImpl(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			VT_CORE_ERROR("File does not exist: {0}", path.string().c_str());
			return nullptr;
		}

		tinygltf::Model gltfInput;
		tinygltf::TinyGLTF gltfContext;

		std::string error, warning;
		bool loaded = false;

		if (path.extension().string() == ".glb")
		{
			loaded = gltfContext.LoadBinaryFromFile(&gltfInput, &error, &warning, path.string());
		}
		else
		{
			loaded = gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, path.string());
		}

		if (!loaded)
		{
			VT_CORE_ERROR("Unable to load GLTF file {0}! Error: {1}, warning {2}", path.string().c_str(), error.c_str(), warning.c_str());
			return nullptr;
		}

		Ref<Mesh> mesh = CreateRef<Mesh>();
		mesh->myMaterial = CreateRef<Material>();
		mesh->myMaterial->myName = path.stem().string() + "_mat";

		uint32_t index = 0;
		for (const auto& mat : gltfInput.materials)
		{
			mesh->myMaterial->mySubMaterials[index] = SubMaterial::Create(mat.name, index, ShaderRegistry::GetShader("Illum"));
			index++;
		}

		const tinygltf::Scene& scene = gltfInput.scenes[gltfInput.defaultScene];
		for (int i : scene.nodes)
		{
			const tinygltf::Node& node = gltfInput.nodes[i];
			LoadNode(node, gltfInput, nullptr, mesh);
		}

		mesh->Construct();

		return mesh;
	}

	void GLTFImporter::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, GLTF::Node* parent, Ref<Mesh> outMesh)
	{
		GLTF::Node node{};

		if (inputNode.translation.size() == 3)
		{
			double dubScale[3] = { inputNode.translation[0], inputNode.translation[1], inputNode.translation[2] };
			node.transform = gem::translate(gem::mat4(1.f), gem::vec3(static_cast<float>(dubScale[0]), static_cast<float>(dubScale[1]), static_cast<float>(dubScale[2])));
		}

		if (inputNode.scale.size() == 3)
		{
			double dubScale[3] = { inputNode.scale[0], inputNode.scale[1], inputNode.scale[2] };
			node.transform = node.transform * gem::scale(gem::mat4(1.f), gem::vec3(static_cast<float>(dubScale[0]), static_cast<float>(dubScale[1]), static_cast<float>(dubScale[2])));
		}

		if (inputNode.rotation.size() == 3)
		{
			double dubRotation[3] = { inputNode.rotation[0], inputNode.rotation[1], inputNode.rotation[2] };
			node.transform = node.transform * gem::rotate(gem::mat4(1.f), static_cast<float>(dubRotation[0]), gem::vec3(1.f, 0.f, 0.f));
			node.transform = node.transform * gem::rotate(gem::mat4(1.f), static_cast<float>(dubRotation[1]), gem::vec3(0.f, 1.f, 0.f));
			node.transform = node.transform * gem::rotate(gem::mat4(1.f), static_cast<float>(dubRotation[2]), gem::vec3(0.f, 0.f, 1.f));
		}

		for (int i : inputNode.children)
		{
			LoadNode(inputModel.nodes[i], inputModel, &node, outMesh);
		}

		if (inputNode.mesh > -1)
		{
			const tinygltf::Mesh mesh = inputModel.meshes[inputNode.mesh];

			for (const tinygltf::Primitive& gltfPrimitive : mesh.primitives)
			{
				uint32_t firstIndex = (uint32_t)outMesh->myIndices.size();
				uint32_t firstVertex = (uint32_t)outMesh->myVertices.size();
				uint32_t indexCount = 0;
				size_t vertexCount = 0;

				// Vertices
				{
					const float* positionBuffer = nullptr;
					const float* normalBuffer = nullptr;
					const float* texCoordsBuffer = nullptr;
					const float* tangentBuffer = nullptr;


					if (gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = inputModel.accessors[gltfPrimitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& view = inputModel.bufferViews[accessor.bufferView];

						positionBuffer = reinterpret_cast<const float*>(&(inputModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						vertexCount = accessor.count;
					}

					if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = inputModel.accessors[gltfPrimitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& view = inputModel.bufferViews[accessor.bufferView];

						normalBuffer = reinterpret_cast<const float*>(&(inputModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = inputModel.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& view = inputModel.bufferViews[accessor.bufferView];

						texCoordsBuffer = reinterpret_cast<const float*>(&(inputModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					if (gltfPrimitive.attributes.find("TANGENT") != gltfPrimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = inputModel.accessors[gltfPrimitive.attributes.find("TANGENT")->second];
						const tinygltf::BufferView& view = inputModel.bufferViews[accessor.bufferView];

						tangentBuffer = reinterpret_cast<const float*>(&(inputModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					for (size_t v = 0; v < vertexCount; v++)
					{
						Vertex vert{};
						vert.position = positionBuffer ? *(gem::vec3*)&positionBuffer[v * 3] : gem::vec3();
						vert.normal = gem::normalize(normalBuffer ? *(gem::vec3*)&normalBuffer[v * 3] : gem::vec3(0.f, 1.f, 0.f));
						vert.texCoords = texCoordsBuffer ? *(gem::vec2*)&texCoordsBuffer[v * 2] : gem::vec2();

						vert.texCoords.y = 1.f - vert.texCoords.y;

						gem::vec4 tangent = tangentBuffer ? *(gem::vec4*)&tangentBuffer[v * 4] : gem::vec4();

						vert.tangent = gem::vec3(tangent.x, tangent.y, tangent.z);

						outMesh->myVertices.emplace_back(vert);
					}
				}

				// Indices
				{
					const tinygltf::Accessor& accessor = inputModel.accessors[gltfPrimitive.indices];
					const tinygltf::BufferView& view = inputModel.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = inputModel.buffers[view.buffer];

					indexCount += static_cast<uint32_t>(accessor.count);

					switch (accessor.componentType)
					{
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
						{
							const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++)
							{
								outMesh->myIndices.emplace_back(buf[index]);
							}

							break;
						}

						case TINYGLTF_PARAMETER_TYPE_SHORT:
						{
							const int16_t* buf = reinterpret_cast<const int16_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++)
							{
								outMesh->myIndices.emplace_back(buf[index]);
							}
							break;
						}

						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
						{
							const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++)
							{
								outMesh->myIndices.emplace_back(buf[index]);
							}
							break;
						}

						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
						{
							const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++)
							{
								outMesh->myIndices.emplace_back(buf[index]);
							}
							break;
						}

						default:
							VT_CORE_ERROR("Index component not supported!");
							return;
					}
				}

				auto& subMesh = outMesh->mySubMeshes.emplace_back();
				subMesh.indexCount = indexCount;
				subMesh.vertexCount = (uint32_t)vertexCount;
				subMesh.indexStartOffset = firstIndex;
				subMesh.vertexStartOffset = firstVertex;
				subMesh.materialIndex = gltfPrimitive.material;
				subMesh.transform = node.transform;
				subMesh.GenerateHash();

				if (!outMesh->myMaterial->mySubMaterials.contains(subMesh.materialIndex))
				{
					outMesh->myMaterial->mySubMaterials[subMesh.materialIndex] = SubMaterial::Create(inputModel.materials[subMesh.materialIndex].name, subMesh.materialIndex, ShaderRegistry::GetShader("Deferred"));
				}
			}
		}
	}
}
