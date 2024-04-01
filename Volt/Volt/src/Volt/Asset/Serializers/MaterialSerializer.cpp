#include "vtpch.h"
#include "MaterialSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/RenderPipeline/ShaderRegistry.h"

namespace Volt
{
	struct SerializedMaterialData
	{
		glm::vec4 color;
		glm::vec3 emissiveColor;
		float emissiveStrength;
		float roughness;
		float metalness;
		float normalStrength;
	};

	struct SerializedTexture
	{
		std::string binding;
		AssetHandle handle;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedTexture& data)
		{
			streamWriter.Write(data.binding);
			streamWriter.Write(data.handle);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedTexture& outData)
		{
			streamReader.Read(outData.binding);
			streamReader.Read(outData.handle);
		}
	};

	struct SerializedShaderDataMember
	{
		std::string name;
		ShaderUniformType type;
		Buffer buffer;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedShaderDataMember& data)
		{
			streamWriter.Write(data.name);
			streamWriter.Write(data.type);
			streamWriter.Write(data.buffer);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedShaderDataMember& outData)
		{
			streamReader.Read(outData.name);
			streamReader.Read(outData.type);
			streamReader.Read(outData.buffer);
		}
	};

	struct SerializedShaderData
	{
		ShaderStage stage;
		size_t size = 0;
		std::vector<SerializedShaderDataMember> members;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedShaderData& data)
		{
			streamWriter.Write(data.stage);
			streamWriter.Write(data.size);
			streamWriter.Write(data.members);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedShaderData& outData)
		{
			streamReader.Read(outData.stage);
			streamReader.Read(outData.size);
			streamReader.Read(outData.members);
		}
	};

	struct SerializedMaterial
	{
		std::string name;
		uint32_t index;
		std::string shaderName;
		bool isPermutation;
		MaterialFlag flags;
		Topology topology;
		CullMode cullMode;
		FillMode fillMode;
		DepthMode depthMode;

		SerializedMaterialData materialData;
		std::vector<SerializedTexture> textures;

		SerializedShaderData specializationData;
		std::vector<SerializedShaderData> pipelineGenerationDatas;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedMaterial& data)
		{
			streamWriter.Write(data.name);
			streamWriter.Write(data.index);
			streamWriter.Write(data.shaderName);
			streamWriter.Write(data.isPermutation);
			streamWriter.Write(data.flags);
			streamWriter.Write(data.topology);
			streamWriter.Write(data.cullMode);
			streamWriter.Write(data.fillMode);
			streamWriter.Write(data.depthMode);
			streamWriter.Write(data.materialData);
			streamWriter.Write(data.textures);
			streamWriter.Write(data.specializationData);
			streamWriter.Write(data.pipelineGenerationDatas);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedMaterial& outData)
		{
			streamReader.Read(outData.name);
			streamReader.Read(outData.index);
			streamReader.Read(outData.shaderName);
			streamReader.Read(outData.isPermutation);
			streamReader.Read(outData.flags);
			streamReader.Read(outData.topology);
			streamReader.Read(outData.cullMode);
			streamReader.Read(outData.fillMode);
			streamReader.Read(outData.depthMode);
			streamReader.Read(outData.materialData);
			streamReader.Read(outData.textures);
			streamReader.Read(outData.specializationData);
			streamReader.Read(outData.pipelineGenerationDatas);
		}
	};

	struct MaterialSerializationData
	{
		std::string name;
		std::vector<SerializedMaterial> materials;

		static void Serialize(BinaryStreamWriter& streamWriter, const MaterialSerializationData& data)
		{
			streamWriter.Write(data.name);
			streamWriter.Write(data.materials);
		}

		static void Deserialize(BinaryStreamReader& streamReader, MaterialSerializationData& outData)
		{
			streamReader.Read(outData.name);
			streamReader.Read(outData.materials);
		}
	};

	template<typename T>
	inline void WriteDataToBuffer(Buffer& buffer, const T& data)
	{
		buffer.Resize(sizeof(T));
		buffer.Copy(&data, sizeof(T));
	}

	void MaterialSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Material> material = std::reinterpret_pointer_cast<Material>(asset);

		MaterialSerializationData serializationData{};
		serializationData.name = material->myName;

		for (const auto& [index, subMaterial] : material->mySubMaterials)
		{
			auto& serMaterial = serializationData.materials.emplace_back();
			serMaterial.name = subMaterial->myName;
			serMaterial.index = index;
			serMaterial.shaderName = subMaterial->myPipeline->GetSpecification().shader->GetName();
			serMaterial.isPermutation = subMaterial->GetPipeline()->IsPermutation();
			serMaterial.flags = subMaterial->myMaterialFlags;
			serMaterial.topology = subMaterial->myTopology;
			serMaterial.cullMode = subMaterial->myCullMode;
			serMaterial.fillMode = subMaterial->myTriangleFillMode;
			serMaterial.depthMode = subMaterial->myDepthMode;

			// Material data
			{
				serMaterial.materialData.color = subMaterial->myMaterialData.color;
				serMaterial.materialData.emissiveColor = subMaterial->myMaterialData.emissiveColor;
				serMaterial.materialData.emissiveStrength = subMaterial->myMaterialData.emissiveStrength;
				serMaterial.materialData.roughness = subMaterial->myMaterialData.roughness;
				serMaterial.materialData.metalness = subMaterial->myMaterialData.metalness;
				serMaterial.materialData.normalStrength = subMaterial->myMaterialData.normalStrength;
			}

			for (const auto& [binding, texture] : subMaterial->myTextures)
			{
				auto& serTexture = serMaterial.textures.emplace_back();
				serTexture.binding = binding;

				AssetHandle textureHandle = Asset::Null();
				if (texture)
				{
					textureHandle = texture->handle;
				}

				serTexture.handle = textureHandle;
			}

			if (subMaterial->GetMaterialSpecializationData().IsValid())
			{
				const auto& materialData = subMaterial->GetMaterialSpecializationData();
				
				serMaterial.specializationData.size = materialData.GetSize();
			
				for (const auto& [memberName, memberData] : materialData.GetMembers())
				{
					auto& serMember = serMaterial.specializationData.members.emplace_back();
					serMember.name = memberName;
					serMember.type = memberData.type;

					switch (memberData.type)
					{
						case ShaderUniformType::Bool: WriteDataToBuffer(serMember.buffer, materialData.GetValue<bool>(memberName)); break;
						case ShaderUniformType::UInt: WriteDataToBuffer(serMember.buffer, materialData.GetValue<uint32_t>(memberName)); break;
						case ShaderUniformType::UInt2: WriteDataToBuffer(serMember.buffer, materialData.GetValue<glm::uvec2>(memberName)); break;
						case ShaderUniformType::UInt3: WriteDataToBuffer(serMember.buffer, materialData.GetValue<glm::uvec3>(memberName)); break;
						case ShaderUniformType::UInt4: WriteDataToBuffer(serMember.buffer, materialData.GetValue<glm::uvec4>(memberName)); break;

						case ShaderUniformType::Int: WriteDataToBuffer(serMember.buffer, materialData.GetValue<int32_t>(memberName)); break;
						case ShaderUniformType::Int2: WriteDataToBuffer(serMember.buffer, materialData.GetValue<glm::ivec2>(memberName)); break;
						case ShaderUniformType::Int3: WriteDataToBuffer(serMember.buffer, materialData.GetValue<glm::ivec3>(memberName)); break;
						case ShaderUniformType::Int4: WriteDataToBuffer(serMember.buffer, materialData.GetValue<glm::ivec4>(memberName)); break;

						case ShaderUniformType::Float: WriteDataToBuffer(serMember.buffer, materialData.GetValue<float>(memberName)); break;
						case ShaderUniformType::Float2: WriteDataToBuffer(serMember.buffer, materialData.GetValue<glm::vec2>(memberName)); break;
						case ShaderUniformType::Float3: WriteDataToBuffer(serMember.buffer, materialData.GetValue<glm::vec3>(memberName)); break;
						case ShaderUniformType::Float4: WriteDataToBuffer(serMember.buffer, materialData.GetValue<glm::vec4>(memberName)); break;
					}
				}
			}

			if (!subMaterial->GetPipelineGenerationDatas().empty())
			{
				for (const auto& [stage, generationData] : subMaterial->GetPipelineGenerationDatas())
				{
					auto& serGenData = serMaterial.pipelineGenerationDatas.emplace_back();
					serGenData.stage = stage;
					serGenData.size = generationData.GetSize();

					for (const auto& [memberName, memberData] : generationData.GetMembers())
					{
						auto& serMember = serMaterial.specializationData.members.emplace_back();
						serMember.name = memberName;
						serMember.type = memberData.type;

						switch (memberData.type)
						{
							case ShaderUniformType::Bool: WriteDataToBuffer(serMember.buffer, generationData.GetValue<bool>(memberName)); break;
							case ShaderUniformType::UInt: WriteDataToBuffer(serMember.buffer, generationData.GetValue<uint32_t>(memberName)); break;
							case ShaderUniformType::UInt2: WriteDataToBuffer(serMember.buffer, generationData.GetValue<glm::uvec2>(memberName)); break;
							case ShaderUniformType::UInt3: WriteDataToBuffer(serMember.buffer, generationData.GetValue<glm::uvec3>(memberName)); break;
							case ShaderUniformType::UInt4: WriteDataToBuffer(serMember.buffer, generationData.GetValue<glm::uvec4>(memberName)); break;

							case ShaderUniformType::Int: WriteDataToBuffer(serMember.buffer, generationData.GetValue<int32_t>(memberName)); break;
							case ShaderUniformType::Int2: WriteDataToBuffer(serMember.buffer, generationData.GetValue<glm::ivec2>(memberName)); break;
							case ShaderUniformType::Int3: WriteDataToBuffer(serMember.buffer, generationData.GetValue<glm::ivec3>(memberName)); break;
							case ShaderUniformType::Int4: WriteDataToBuffer(serMember.buffer, generationData.GetValue<glm::ivec4>(memberName)); break;

							case ShaderUniformType::Float: WriteDataToBuffer(serMember.buffer, generationData.GetValue<float>(memberName)); break;
							case ShaderUniformType::Float2: WriteDataToBuffer(serMember.buffer, generationData.GetValue<glm::vec2>(memberName)); break;
							case ShaderUniformType::Float3: WriteDataToBuffer(serMember.buffer, generationData.GetValue<glm::vec3>(memberName)); break;
							case ShaderUniformType::Float4: WriteDataToBuffer(serMember.buffer, generationData.GetValue<glm::vec4>(memberName)); break;
						}
					}
				}
			}
		}

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);
		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	template<typename T>
	void ReadDataFromBuffer(const Buffer& buffer, T& outData)
	{
		memcpy_s(&outData, sizeof(T), buffer.As<void>(), buffer.GetSize());
	}

	bool MaterialSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		VT_CORE_ASSERT(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		MaterialSerializationData serializationData{};
		streamReader.Read(serializationData);

		std::unordered_map<uint32_t, Ref<SubMaterial>> materials;

		for (const auto& subMaterial : serializationData.materials)
		{
			Ref<Shader> shader = ShaderRegistry::GetShader(subMaterial.shaderName);
			if (!shader || !shader->IsValid())
			{
				shader = Renderer::GetDefaultData().defaultShader;
				VT_CORE_ERROR("Shader {0} not found or invalid! Falling back to default!", subMaterial.shaderName);
			}

			Ref<SubMaterial> material = SubMaterial::Create(subMaterial.name, subMaterial.index, shader);

			material->SetFlags(subMaterial.flags);
			if (!material->HasFlag(MaterialFlag::Opaque) && !material->HasFlag(MaterialFlag::Transparent) && !material->HasFlag(MaterialFlag::Deferred))
			{
				material->SetFlag(MaterialFlag::Deferred, true);
			}

			if (subMaterial.shaderName == "Illum" && material->HasFlag(MaterialFlag::Opaque))
			{
				material->SetFlag(MaterialFlag::Deferred, true);
				material->SetFlag(MaterialFlag::Opaque, false);
			}

			material->myTopology = subMaterial.topology;
			material->myCullMode = subMaterial.cullMode;
			material->myTriangleFillMode = subMaterial.fillMode;
			material->myDepthMode = subMaterial.depthMode;

			material->InvalidatePipeline(shader);

			material->myMaterialData.color = subMaterial.materialData.color;
			material->myMaterialData.emissiveColor = subMaterial.materialData.emissiveColor;
			material->myMaterialData.emissiveStrength = subMaterial.materialData.emissiveStrength;
			material->myMaterialData.roughness = subMaterial.materialData.roughness;
			material->myMaterialData.metalness = subMaterial.materialData.metalness;
			material->myMaterialData.normalStrength = subMaterial.materialData.normalStrength;
			
			std::unordered_map<std::string, Ref<Texture2D>> textures;
			for (const auto& serTexture : subMaterial.textures)
			{
				Ref<Texture2D> texture = AssetManager::QueueAsset<Texture2D>(serTexture.handle);
				if (texture)
				{
					textures.emplace(serTexture.binding, texture);
				}
				else
				{
					textures.emplace(serTexture.binding, nullptr);
				}
			}

			for (const auto& [shaderName, texture] : textures)
			{
				const auto& textureDefinitions = shader->GetResources().shaderTextureDefinitions;

				bool isDefault = shaderName == "albedo" || shaderName == "normal" || shaderName == "material";

				if (auto it = std::find_if(textureDefinitions.begin(), textureDefinitions.end(), [&](const auto& lhs)
				{
					return lhs.shaderName == shaderName;

				}); it != textureDefinitions.end() || isDefault)
				{
					material->SetTexture(shaderName, texture);
				}
			}

			if (material->GetMaterialSpecializationData().IsValid())
			{
				auto& materialData = material->GetMaterialSpecializationData();
				
				for (const auto& member : subMaterial.specializationData.members)
				{
					auto it = std::find_if(materialData.GetMembers().begin(), materialData.GetMembers().end(), [&](const auto& value)
					{
						return value.first == member.name && value.second.type == member.type;
					});

					if (it == materialData.GetMembers().end())
					{
						continue;
					}

					switch (member.type)
					{
						case ShaderUniformType::Bool: ReadDataFromBuffer(member.buffer, materialData.GetValue<bool>(member.name)); break;
						case ShaderUniformType::UInt: ReadDataFromBuffer(member.buffer, materialData.GetValue<uint32_t>(member.name)); break;
						case ShaderUniformType::UInt2: ReadDataFromBuffer(member.buffer, materialData.GetValue<glm::uvec2>(member.name)); break;
						case ShaderUniformType::UInt3: ReadDataFromBuffer(member.buffer, materialData.GetValue<glm::uvec3>(member.name)); break;
						case ShaderUniformType::UInt4: ReadDataFromBuffer(member.buffer, materialData.GetValue<glm::uvec4>(member.name)); break;

						case ShaderUniformType::Int: ReadDataFromBuffer(member.buffer, materialData.GetValue<int32_t>(member.name)); break;
						case ShaderUniformType::Int2: ReadDataFromBuffer(member.buffer, materialData.GetValue<glm::ivec2>(member.name)); break;
						case ShaderUniformType::Int3: ReadDataFromBuffer(member.buffer, materialData.GetValue<glm::ivec3>(member.name)); break;
						case ShaderUniformType::Int4: ReadDataFromBuffer(member.buffer, materialData.GetValue<glm::ivec4>(member.name)); break;

						case ShaderUniformType::Float: ReadDataFromBuffer(member.buffer, materialData.GetValue<float>(member.name)); break;
						case ShaderUniformType::Float2: ReadDataFromBuffer(member.buffer, materialData.GetValue<glm::vec2>(member.name)); break;
						case ShaderUniformType::Float3: ReadDataFromBuffer(member.buffer, materialData.GetValue<glm::vec3>(member.name)); break;
						case ShaderUniformType::Float4: ReadDataFromBuffer(member.buffer, materialData.GetValue<glm::vec4>(member.name)); break;
					}
				}
			}

			if (subMaterial.isPermutation && !material->GetPipelineGenerationDatas().empty())
			{
				std::map<ShaderStage, ShaderDataBuffer>& generationData = material->GetPipelineGenerationDatas();

				for (const auto& serGenData : subMaterial.pipelineGenerationDatas)
				{
					if (!generationData.contains(serGenData.stage))
					{
						continue;
					}

					for (const auto& member : serGenData.members)
					{
						auto it = std::find_if(generationData.at(serGenData.stage).GetMembers().begin(), generationData.at(serGenData.stage).GetMembers().end(), [&](const auto& value)
						{
							return value.first == member.name && value.second.type == member.type;
						});

						if (it == generationData.at(serGenData.stage).GetMembers().end())
						{
							continue;
						}

						switch (member.type)
						{
							case ShaderUniformType::Bool: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<bool>(member.name)); break;
							case ShaderUniformType::UInt: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<uint32_t>(member.name)); break;
							case ShaderUniformType::UInt2: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<glm::uvec2>(member.name)); break;
							case ShaderUniformType::UInt3: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<glm::uvec3>(member.name)); break;
							case ShaderUniformType::UInt4: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<glm::uvec4>(member.name)); break;

							case ShaderUniformType::Int: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<int32_t>(member.name)); break;
							case ShaderUniformType::Int2: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<glm::ivec2>(member.name)); break;
							case ShaderUniformType::Int3: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<glm::ivec3>(member.name)); break;
							case ShaderUniformType::Int4: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<glm::ivec4>(member.name)); break;

							case ShaderUniformType::Float: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<float>(member.name)); break;
							case ShaderUniformType::Float2: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<glm::vec2>(member.name)); break;
							case ShaderUniformType::Float3: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<glm::vec3>(member.name)); break;
							case ShaderUniformType::Float4: ReadDataFromBuffer(member.buffer, generationData.at(serGenData.stage).GetValue<glm::vec4>(member.name)); break;
						}
					}
				}

				material->RecompilePermutation();
			}

			Renderer::UpdateMaterial(material.get());
			materials.emplace(subMaterial.index, material);
		}

		Ref<Material> material = std::reinterpret_pointer_cast<Material>(destinationAsset);
		material->myName = serializationData.name;
		material->mySubMaterials = materials;

		return true;
	}
}
