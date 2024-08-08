#include "vtpch.h"
#include "AssetImporter.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Importers/TextureImporter.h"
#include "Volt/Asset/Text/Font.h"

#include "Volt/Asset/Rendering/ShaderDefinition.h"

#include "Volt/Animation/BlendSpace.h"

#include "Volt/Physics/PhysicsMaterial.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include "Volt/Rendering/Shader/ShaderMap.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <RHIModule/Shader/Shader.h>

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>
#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>

#include <yaml-cpp/yaml.h>

namespace Volt
{
	bool TextureSourceImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		Ref<Texture2D> texture = std::reinterpret_pointer_cast<Texture2D>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(LogVerbosity::Error, "File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		if (!TextureImporter::ImportTexture(filePath, *texture))
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		return true;
	}

	void TextureSourceImporter::Save(const AssetMetadata&, const Ref<Asset>&) const
	{
	}

	bool ShaderDefinitionImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		return false;
	}

	void ShaderDefinitionImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
	}

	//bool MaterialImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	//{
	//	asset = CreateRef<Material>();
	//	const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

	//	if (!std::filesystem::exists(filePath))
	//	{
	//		VT_LOG(LogSeverity::Error, "File {0} not found!", metadata.filePath);
	//		asset->SetFlag(AssetFlag::Missing, true);
	//		return false;
	//	}

	//	std::ifstream file(filePath);
	//	if (!file.is_open())
	//	{
	//		VT_LOG(LogSeverity::Error, "Failed to open file: {0}!", metadata.filePath);
	//		asset->SetFlag(AssetFlag::Invalid, true);
	//		return false;
	//	}

	//	std::stringstream sstream;
	//	sstream << file.rdbuf();
	//	file.close();

	//	YAML::Node root;

	//	try
	//	{
	//		root = YAML::Load(sstream.str());
	//	}
	//	catch (std::exception& e)
	//	{
	//		VT_LOG(LogSeverity::Error, "{0} contains invalid YAML! Please correct it! Error: {1}", metadata.filePath, e.what());
	//		asset->SetFlag(AssetFlag::Invalid, true);
	//		return false;
	//	}

	//	YAML::Node rootMaterialNode = root["Material"];

	//	std::string nameString;
	//	VT_DESERIALIZE_PROPERTY(name, nameString, rootMaterialNode, std::string("Null"));

	//	std::unordered_map<uint32_t, Ref<SubMaterial>> materials;

	//	YAML::Node materialsNode = rootMaterialNode["materials"];
	//	for (const auto& materialNode : materialsNode)
	//	{
	//		std::string materialNameString;
	//		VT_DESERIALIZE_PROPERTY(material, materialNameString, materialNode, std::string("Null"));

	//		uint32_t materialIndex;
	//		VT_DESERIALIZE_PROPERTY(index, materialIndex, materialNode, 0);

	//		std::string shaderNameString;
	//		VT_DESERIALIZE_PROPERTY(shader, shaderNameString, materialNode, std::string("Illum"));

	//		uint32_t materialFlags;
	//		VT_DESERIALIZE_PROPERTY(flags, materialFlags, materialNode, (uint32_t)(MaterialFlag::Deferred | MaterialFlag::CastAO | MaterialFlag::CastShadows));

	//		bool isPermutation;
	//		VT_DESERIALIZE_PROPERTY(isPermutation, isPermutation, materialNode, false);

	//		std::map<std::string, Ref<Texture2D>> textures;

	//		YAML::Node texturesNode = materialNode["textures"];
	//		for (const auto& textureNode : texturesNode)
	//		{
	//			std::string shaderName;
	//			VT_DESERIALIZE_PROPERTY(binding, shaderName, textureNode, std::string("Empty"));

	//			AssetHandle textureHandle;
	//			VT_DESERIALIZE_PROPERTY(handle, textureHandle, textureNode, uint64_t(0));

	//			Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);
	//			if (texture)
	//			{
	//				textures.emplace(shaderName, texture);
	//			}
	//		}

	//		RefPtr<RHI::Shader> shader = ShaderMap::Get("VisibilityBuffer");
	//		if (!shader)
	//		{
	//			shader = ShaderMap::Get("VisibilityBuffer");
	//			VT_LOG(LogSeverity::Error, "Shader {0} not found or invalid! Falling back to default!", shaderNameString);
	//		}

	//		Ref<SubMaterial> material = SubMaterial::Create(materialNameString, materialIndex, shader);

	//		for (const auto& [name, texture] : textures)
	//		{
	//			material->AddTexture(texture);
	//		}

	//		//material->SetFlags((MaterialFlag)materialFlags);
	//		//if (!material->HasFlag(MaterialFlag::Opaque) && !material->HasFlag(MaterialFlag::Transparent) && !material->HasFlag(MaterialFlag::Deferred))
	//		//{
	//		//	material->SetFlag(MaterialFlag::Deferred, true);
	//		//}

	//		//if (shaderNameString == "Illum" && material->HasFlag(MaterialFlag::Opaque))
	//		//{
	//		//	material->SetFlag(MaterialFlag::Deferred, true);
	//		//	material->SetFlag(MaterialFlag::Opaque, false);
	//		//}

	//		//// Get pipeline properties
	//		//{
	//		//	Topology topology;
	//		//	CullMode cullMode;
	//		//	FillMode triangleFillMode;
	//		//	DepthMode depthMode;

	//		//	VT_DESERIALIZE_PROPERTY(topology, *(uint32_t*)&topology, materialNode, (uint32_t)Topology::TriangleList);
	//		//	VT_DESERIALIZE_PROPERTY(cullMode, *(uint32_t*)&cullMode, materialNode, (uint32_t)CullMode::Back);
	//		//	VT_DESERIALIZE_PROPERTY(triangleFillMode, *(uint32_t*)&triangleFillMode, materialNode, (uint32_t)FillMode::Solid);
	//		//	VT_DESERIALIZE_PROPERTY(depthMode, *(uint32_t*)&depthMode, materialNode, (uint32_t)DepthMode::ReadWrite);

	//		//	material->myTopology = topology;
	//		//	material->myCullMode = cullMode;
	//		//	material->myTriangleFillMode = triangleFillMode;
	//		//	material->myDepthMode = depthMode;

	//		//	material->InvalidatePipeline(shader);
	//		//}


	//		//auto materialDataNode = materialNode["data"];
	//		//if (materialDataNode)
	//		//{
	//		//	VT_DESERIALIZE_PROPERTY(color, material->myMaterialData.color, materialDataNode, glm::vec4(1.f));
	//		//	VT_DESERIALIZE_PROPERTY(emissiveColor, material->myMaterialData.emissiveColor, materialDataNode, glm::vec3(1.f));
	//		//	VT_DESERIALIZE_PROPERTY(emissiveStrength, material->myMaterialData.emissiveStrength, materialDataNode, 1.f);
	//		//	VT_DESERIALIZE_PROPERTY(roughness, material->myMaterialData.roughness, materialDataNode, 0.5f);
	//		//	VT_DESERIALIZE_PROPERTY(metalness, material->myMaterialData.metalness, materialDataNode, 0.f);
	//		//	VT_DESERIALIZE_PROPERTY(normalStrength, material->myMaterialData.normalStrength, materialDataNode, 0.f);
	//		//}

	//		//for (const auto& [shaderName, texture] : textures)
	//		//{
	//		//	const auto& textureDefinitions = shader->GetResources().shaderTextureDefinitions;

	//		//	bool isDefault = shaderName == "albedo" || shaderName == "normal" || shaderName == "material";

	//		//	if (auto it = std::find_if(textureDefinitions.begin(), textureDefinitions.end(), [&](const auto& lhs)
	//		//	{
	//		//		return lhs.shaderName == shaderName;

	//		//	}); it != textureDefinitions.end() || isDefault)
	//		//	{
	//		//		material->SetTexture(shaderName, texture);
	//		//	}
	//		//}

	//		//YAML::Node specializationDataNode = materialNode["specializationData"];
	//		//if (specializationDataNode && material->GetMaterialSpecializationData().IsValid())
	//		//{
	//		//	auto& materialData = material->GetMaterialSpecializationData();

	//		//	for (const auto& memberNode : specializationDataNode["members"])
	//		//	{
	//		//		std::string memberName;
	//		//		VT_DESERIALIZE_PROPERTY(name, memberName, memberNode, std::string(""));

	//		//		ShaderUniformType type;
	//		//		VT_DESERIALIZE_PROPERTY(type, type, memberNode, ShaderUniformType::Bool);

	//		//		auto it = std::find_if(materialData.GetMembers().begin(), materialData.GetMembers().end(), [&](const auto& value)
	//		//		{
	//		//			return value.first == memberName && value.second.type == type;
	//		//		});

	//		//		if (it != materialData.GetMembers().end())
	//		//		{
	//		//			switch (type)
	//		//			{
	//		//				case Volt::ShaderUniformType::Bool: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<bool>(memberName), memberNode, false); break;
	//		//				case Volt::ShaderUniformType::UInt: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<uint32_t>(memberName), memberNode, 0u); break;
	//		//				case Volt::ShaderUniformType::UInt2: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<glm::uvec2>(memberName), memberNode, glm::uvec2{ 0 }); break;
	//		//				case Volt::ShaderUniformType::UInt3: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<glm::uvec3>(memberName), memberNode, glm::uvec3{ 0 }); break;
	//		//				case Volt::ShaderUniformType::UInt4: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<glm::uvec4>(memberName), memberNode, glm::uvec4{ 0 }); break;

	//		//				case Volt::ShaderUniformType::Int: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<int32_t>(memberName), memberNode, 0); break;
	//		//				case Volt::ShaderUniformType::Int2: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<glm::ivec2>(memberName), memberNode, glm::ivec2{ 0 }); break;
	//		//				case Volt::ShaderUniformType::Int3: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<glm::ivec3>(memberName), memberNode, glm::ivec3{ 0 }); break;
	//		//				case Volt::ShaderUniformType::Int4: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<glm::ivec4>(memberName), memberNode, glm::ivec4{ 0 }); break;

	//		//				case Volt::ShaderUniformType::Float: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<float>(memberName), memberNode, 0.f); break;
	//		//				case Volt::ShaderUniformType::Float2: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<glm::vec2>(memberName), memberNode, glm::vec2{ 0.f }); break;
	//		//				case Volt::ShaderUniformType::Float3: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<glm::vec3>(memberName), memberNode, glm::vec3{ 0.f }); break;
	//		//				case Volt::ShaderUniformType::Float4: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<glm::vec4>(memberName), memberNode, glm::vec4{ 0.f }); break;
	//		//			}
	//		//		}
	//		//	}
	//		//}

	//		//YAML::Node pipelineGenerationNode = materialNode["pipelineGenerationData"];
	//		//if (isPermutation && pipelineGenerationNode && !material->GetPipelineGenerationDatas().empty())
	//		//{
	//		//	std::map<ShaderStage, ShaderDataBuffer>& generationData = material->GetPipelineGenerationDatas();

	//		//	for (const auto& generationDataNode : pipelineGenerationNode)
	//		//	{
	//		//		ShaderStage stage;
	//		//		VT_DESERIALIZE_PROPERTY(stage, stage, generationDataNode, ShaderStage::None);

	//		//		if (!generationData.contains(stage))
	//		//		{
	//		//			continue;
	//		//		}

	//		//		for (const auto& memberNode : generationDataNode["members"])
	//		//		{
	//		//			std::string memberName;
	//		//			VT_DESERIALIZE_PROPERTY(name, memberName, memberNode, std::string(""));

	//		//			ShaderUniformType type;
	//		//			VT_DESERIALIZE_PROPERTY(type, type, memberNode, ShaderUniformType::Bool);

	//		//			auto it = std::find_if(generationData.at(stage).GetMembers().begin(), generationData.at(stage).GetMembers().end(), [&](const auto& value)
	//		//			{
	//		//				return value.first == memberName && value.second.type == type;
	//		//			});

	//		//			if (it != generationData.at(stage).GetMembers().end())
	//		//			{
	//		//				switch (type)
	//		//				{
	//		//					case Volt::ShaderUniformType::Bool: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<bool>(memberName), memberNode, false); break;
	//		//					case Volt::ShaderUniformType::UInt: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<uint32_t>(memberName), memberNode, 0u); break;
	//		//					case Volt::ShaderUniformType::UInt2: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<glm::uvec2>(memberName), memberNode, glm::uvec2{ 0 }); break;
	//		//					case Volt::ShaderUniformType::UInt3: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<glm::uvec3>(memberName), memberNode, glm::uvec3{ 0 }); break;
	//		//					case Volt::ShaderUniformType::UInt4: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<glm::uvec4>(memberName), memberNode, glm::uvec4{ 0 }); break;

	//		//					case Volt::ShaderUniformType::Int: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<int32_t>(memberName), memberNode, 0); break;
	//		//					case Volt::ShaderUniformType::Int2: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<glm::ivec2>(memberName), memberNode, glm::ivec2{ 0 }); break;
	//		//					case Volt::ShaderUniformType::Int3: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<glm::ivec3>(memberName), memberNode, glm::ivec3{ 0 }); break;
	//		//					case Volt::ShaderUniformType::Int4: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<glm::ivec4>(memberName), memberNode, glm::ivec4{ 0 }); break;

	//		//					case Volt::ShaderUniformType::Float: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<float>(memberName), memberNode, 0.f); break;
	//		//					case Volt::ShaderUniformType::Float2: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<glm::vec2>(memberName), memberNode, glm::vec2{ 0.f }); break;
	//		//					case Volt::ShaderUniformType::Float3: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<glm::vec3>(memberName), memberNode, glm::vec3{ 0.f }); break;
	//		//					case Volt::ShaderUniformType::Float4: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<glm::vec4>(memberName), memberNode, glm::vec4{ 0.f }); break;
	//		//				}
	//		//			}
	//		//		}
	//		//	}

	//		//	material->RecompilePermutation();
	//		//}

	//		//Renderer::UpdateMaterial(material.get());

	//		materials.emplace(materialIndex, material);
	//	}

	//	Ref<Material> material = std::reinterpret_pointer_cast<Material>(asset);
	//	material->myName = nameString;
	//	material->mySubMaterials = materials;

	//	return true;
	//}

	//void MaterialImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	//{
	//	Ref<Material> material = std::reinterpret_pointer_cast<Material>(asset);

	//	YAML::Emitter out;
	//	out << YAML::BeginMap;
	//	out << YAML::Key << "Material" << YAML::Value;
	//	{
	//		out << YAML::BeginMap;
	//		VT_SERIALIZE_PROPERTY(name, material->myName, out);
	//		{
	//			out << YAML::Key << "materials" << YAML::BeginSeq;
	//			for (const auto& [index, subMaterial] : material->mySubMaterials)
	//			{
	//				out << YAML::BeginMap;
	//				VT_SERIALIZE_PROPERTY(material, subMaterial->m_name, out);
	//				VT_SERIALIZE_PROPERTY(index, index, out);
	//				VT_SERIALIZE_PROPERTY(shader, subMaterial->m_pipeline->GetShader()->GetName(), out);
	//				VT_SERIALIZE_PROPERTY(flags, (uint32_t)subMaterial->m_materialFlags, out);
	//				VT_SERIALIZE_PROPERTY(topology, (uint32_t)subMaterial->m_topology, out);
	//				VT_SERIALIZE_PROPERTY(cullMode, (uint32_t)subMaterial->m_cullMode, out);
	//				VT_SERIALIZE_PROPERTY(triangleFillMode, (uint32_t)subMaterial->m_triangleFillMode, out);
	//				VT_SERIALIZE_PROPERTY(depthMode, (uint32_t)subMaterial->m_depthMode, out);

	//				out << YAML::Key << "textures" << YAML::BeginSeq;
	//				for (const auto& texture : subMaterial->m_textures)
	//				{
	//					std::string binding = "";

	//					out << YAML::BeginMap;
	//					VT_SERIALIZE_PROPERTY(binding, binding, out);

	//					AssetHandle textureHandle = Asset::Null();
	//					if (texture)
	//					{
	//						textureHandle = texture->handle;
	//					}

	//					VT_SERIALIZE_PROPERTY(handle, textureHandle, out);
	//					out << YAML::EndMap;
	//				}
	//				out << YAML::EndSeq;
	//				out << YAML::EndMap;
	//			}
	//			out << YAML::EndSeq;
	//		}

	//		out << YAML::EndMap;
	//	}
	//	out << YAML::EndMap;

	//	std::ofstream fout(AssetManager::GetFilesystemPath(metadata.filePath));
	//	fout << out.c_str();
	//	fout.close();
	//}

	bool FontImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<Font>();
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(LogVerbosity::Error, "File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		asset = CreateRef<Font>();
		std::reinterpret_pointer_cast<Font>(asset)->Initialize(filePath);
		return true;
	}

	bool PhysicsMaterialImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<PhysicsMaterial>();
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(LogVerbosity::Error, "File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLFileStreamReader streamReader{};
		if (!streamReader.OpenFile(filePath))
		{
			VT_LOG(LogVerbosity::Error, "Failed to open file: {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		Ref<PhysicsMaterial> physicsMat = std::reinterpret_pointer_cast<PhysicsMaterial>(asset);

		physicsMat->staticFriction = streamReader.ReadAtKey("staticFriction", 0.1f);
		physicsMat->dynamicFriction = streamReader.ReadAtKey("dynamicFriction", 0.1f);
		physicsMat->bounciness = streamReader.ReadAtKey("bounciness", 0.1f);

		return false;
	}

	void PhysicsMaterialImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<PhysicsMaterial> material = std::reinterpret_pointer_cast<PhysicsMaterial>(asset);

		YAMLFileStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };

		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("PhysicsMaterial");

		streamWriter.SetKey("staticFriction", material->staticFriction);
		streamWriter.SetKey("dynamicFriction", material->dynamicFriction);
		streamWriter.SetKey("bounciness", material->bounciness);

		streamWriter.EndMap();
		streamWriter.EndMap();

		streamWriter.WriteToDisk();
	}

	bool BlendSpaceImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<BlendSpace>();
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(LogVerbosity::Error, "File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLFileStreamReader streamReader{};

		if (!streamReader.OpenFile(filePath))
		{
			VT_LOG(LogVerbosity::Error, "Failed to open file: {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}


		uint32_t dimension = 0;
		glm::vec2 horizontalValues;
		glm::vec2 verticalValues;
		Vector<std::pair<glm::vec2, AssetHandle>> animations;

		streamReader.EnterScope("BlendSpace");

		dimension = streamReader.ReadAtKey("dimension", 0u);
		horizontalValues = streamReader.ReadAtKey("horizontalValues", glm::vec2{ -1.f, 1.f });
		verticalValues = streamReader.ReadAtKey("verticalValues", glm::vec2{ -1.f, 1.f });

		streamReader.ForEach("Animations", [&]() 
		{
			auto& [value, anim] = animations.emplace_back();
			anim = streamReader.ReadAtKey("animation", AssetHandle(0));
			value = streamReader.ReadAtKey("value", glm::vec2(0.f));
		});

		streamReader.ExitScope();

		Ref<BlendSpace> blendSpace = std::reinterpret_pointer_cast<BlendSpace>(asset);
		blendSpace->myAnimations = animations;
		blendSpace->myDimension = (BlendSpaceDimension)dimension;
		blendSpace->myHorizontalValues = horizontalValues;
		blendSpace->myVerticalValues = verticalValues;

		return true;
	}

	void BlendSpaceImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<BlendSpace> blendSpace = std::reinterpret_pointer_cast<BlendSpace>(asset);

		YAMLFileStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };

		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("BlendSpace");

		streamWriter.SetKey("dimension", (uint32_t)blendSpace->myDimension);
		streamWriter.SetKey("horizontalValues", blendSpace->myHorizontalValues);
		streamWriter.SetKey("verticalValues", blendSpace->myVerticalValues);

		streamWriter.BeginSequence("Animations");
		for (const auto& anim : blendSpace->myAnimations)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("animation", anim.second);
			streamWriter.SetKey("value", anim.first);
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.EndMap();
		streamWriter.EndMap();
	
		streamWriter.WriteToDisk();
	}
}
