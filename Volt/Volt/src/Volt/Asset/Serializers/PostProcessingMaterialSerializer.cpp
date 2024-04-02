#include "vtpch.h"
#include "PostProcessingMaterialSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Rendering/PostProcessingMaterial.h"

#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/RenderingNew/Shader/ShaderMap.h"
#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	struct SerializedMember
	{
		std::string name;
		ShaderUniformType type;
		Buffer buffer;

		static void Serialize(BinaryStreamWriter& streamWriter, const SerializedMember& data)
		{
			streamWriter.Write(data.name);
			streamWriter.Write(data.type);
			streamWriter.Write(data.buffer);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SerializedMember& outData)
		{
			streamReader.Read(outData.name);
			streamReader.Read(outData.type);
			streamReader.Read(outData.buffer);
		}
	};

	struct SpecializationData
	{
		size_t size;
		std::vector<SerializedMember> members;

		static void Serialize(BinaryStreamWriter& streamWriter, const SpecializationData& data)
		{
			streamWriter.Write(data.size);
			streamWriter.Write(data.members);
		}

		static void Deserialize(BinaryStreamReader& streamReader, SpecializationData& outData)
		{
			streamReader.Read(outData.size);
			streamReader.Read(outData.members);
		}
	};

	struct PostProcessingMaterialSerializationData
	{
		std::string shaderName;
		SpecializationData specializationData;

		static void Serialize(BinaryStreamWriter& streamWriter, const PostProcessingMaterialSerializationData& data)
		{
			streamWriter.Write(data.shaderName);
			streamWriter.Write(data.specializationData);
		}

		static void Deserialize(BinaryStreamReader& streamReader, PostProcessingMaterialSerializationData& outData)
		{
			streamReader.Read(outData.shaderName);
			streamReader.Read(outData.specializationData);
		}
	};

	template<typename T>
	inline void WriteDataToBuffer(Buffer& buffer, const T& data)
	{
		buffer.Resize(sizeof(T));
		buffer.Copy(&data, sizeof(T));
	}

	void PostProcessingMaterialSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<PostProcessingMaterial> material = std::reinterpret_pointer_cast<PostProcessingMaterial>(asset);

		PostProcessingMaterialSerializationData serializationData{};

		serializationData.shaderName = material->myPipeline->GetShader()->GetName();

		if (material->myMaterialData.IsValid())
		{
			const auto& materialData = material->myMaterialData;
			serializationData.specializationData.size = materialData.GetSize();

			for (const auto& [memberName, memberData] : materialData.GetMembers())
			{
				auto& newMember = serializationData.specializationData.members.emplace_back();
				newMember.name = memberName;
				newMember.type = memberData.type;
				
				switch (memberData.type)
				{
					case ShaderUniformType::Bool: WriteDataToBuffer(newMember.buffer, materialData.GetValue<bool>(memberName)); break;
					case ShaderUniformType::UInt: WriteDataToBuffer(newMember.buffer, materialData.GetValue<uint32_t>(memberName)); break;
					case ShaderUniformType::UInt2: WriteDataToBuffer(newMember.buffer, materialData.GetValue<glm::uvec2>(memberName)); break;
					case ShaderUniformType::UInt3: WriteDataToBuffer(newMember.buffer, materialData.GetValue<glm::uvec3>(memberName)); break;
					case ShaderUniformType::UInt4: WriteDataToBuffer(newMember.buffer, materialData.GetValue<glm::uvec4>(memberName)); break;

					case ShaderUniformType::Int: WriteDataToBuffer(newMember.buffer, materialData.GetValue<int32_t>(memberName)); break;
					case ShaderUniformType::Int2: WriteDataToBuffer(newMember.buffer, materialData.GetValue<glm::ivec2>(memberName)); break;
					case ShaderUniformType::Int3: WriteDataToBuffer(newMember.buffer, materialData.GetValue<glm::ivec3>(memberName)); break;
					case ShaderUniformType::Int4: WriteDataToBuffer(newMember.buffer, materialData.GetValue<glm::ivec4>(memberName)); break;

					case ShaderUniformType::Float: WriteDataToBuffer(newMember.buffer, materialData.GetValue<float>(memberName)); break;
					case ShaderUniformType::Float2: WriteDataToBuffer(newMember.buffer, materialData.GetValue<glm::vec2>(memberName)); break;
					case ShaderUniformType::Float3: WriteDataToBuffer(newMember.buffer, materialData.GetValue<glm::vec3>(memberName)); break;
					case ShaderUniformType::Float4: WriteDataToBuffer(newMember.buffer, materialData.GetValue<glm::vec4>(memberName)); break;
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

	bool PostProcessingMaterialSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
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
		VT_CORE_ASSERT(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!")

		PostProcessingMaterialSerializationData serializationData{};
		streamReader.Read(serializationData);

		if (serializationData.shaderName.empty())
		{
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		Ref<Shader> shader;
		//shader = ShaderRegistry::GetShader(serializationData.shaderName);
		if (!shader)
		{
			shader = Renderer::GetDefaultData().defaultPostProcessingShader;
		}

		Ref<PostProcessingMaterial> material = std::reinterpret_pointer_cast<PostProcessingMaterial>(destinationAsset);
		material->Initialize(shader);

		if (material->myMaterialData.IsValid())
		{
			auto& materialData = material->myMaterialData;

			for (const auto& member : serializationData.specializationData.members)
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

		return true;
	}
}
