#include "vtpch.h"
#include "AssetImporter.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Importers/TextureImporter.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/Text/Font.h"
#include "Volt/Asset/Video/Video.h"

#include "Volt/Asset/Rendering/PostProcessingStack.h"
#include "Volt/Asset/Rendering/PostProcessingMaterial.h"

#include "Volt/Animation/BlendSpace.h"

#include "Volt/Physics/PhysicsMaterial.h"

#include "Volt/Rendering/RenderPipeline/ShaderRegistry.h"
#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Texture/TextureTable.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Shader/Shader.h"

#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <yaml-cpp/yaml.h>

namespace Volt
{
	bool TextureSourceImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Texture2D>();
		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}
		auto mesh = TextureImporter::ImportTexture(filePath);

		if (!mesh) [[unlikely]]
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		asset = mesh;
		asset->path = path;

		Renderer::AddTexture(std::reinterpret_pointer_cast<Texture2D>(asset)->GetImage());
		return true;
	}

	void TextureSourceImporter::Save(const Ref<Asset>&) const
	{
	}

	void TextureSourceImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{
	}

	bool TextureSourceImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}

	bool ShaderImporter::Load(const std::filesystem::path& inPath, Ref<Asset>& asset) const
	{
		asset = CreateRef<Shader>();
		const std::filesystem::path filesytemPath = AssetManager::GetContextPath(inPath) / inPath;

		if (!std::filesystem::exists(filesytemPath)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", inPath.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filesytemPath);
		if (!file.is_open()) [[unlikely]]
		{
			VT_CORE_ERROR("Failed to open file: {0}!", inPath.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", inPath, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::string name = root["name"] ? root["name"].as<std::string>() : "Unnamed";

		bool isInternal;
		VT_DESERIALIZE_PROPERTY(internal, isInternal, root, false);

		if (!root["paths"]) [[unlikely]]
		{
			VT_CORE_ERROR("No shaders defined in shader definition {0}!", inPath.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node pathsNode = root["paths"];
		std::vector<std::filesystem::path> paths;
		for (const auto path : pathsNode)
		{
			paths.emplace_back(path.as<std::string>());
		}

		YAML::Node inputTexturesNode = root["inputTextures"];
		std::vector<ShaderTexture> inputTextures; // shader name -> editor name
		for (const auto input : inputTexturesNode)
		{
			std::string shaderName;
			VT_DESERIALIZE_PROPERTY(shaderName, shaderName, input, std::string("Empty"));

			std::string editorName;
			VT_DESERIALIZE_PROPERTY(editorName, editorName, input, std::string("Null"));

			inputTextures.emplace_back(shaderName, editorName);
		}

		Ref<Shader> shader = Shader::Create(name, paths, false);
		const_cast<std::vector<ShaderTexture>&>(shader->GetResources().shaderTextureDefinitions) = inputTextures; // #TODO_Ivar: Add checking
		shader->myIsInternal = isInternal;

		if (!shader) [[unlikely]]
		{
			VT_CORE_ERROR("Failed to create shader {0}!", name.c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		asset = shader;
		asset->path = inPath;

		return true;
	}

	void ShaderImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<Shader> shader = std::reinterpret_pointer_cast<Shader>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		VT_SERIALIZE_PROPERTY(name, shader->GetName(), out);
		VT_SERIALIZE_PROPERTY(internal, shader->IsInternal(), out);

		out << YAML::Key << "paths" << YAML::BeginSeq;
		for (const auto& path : shader->GetSourcePaths())
		{
			out << path;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "inputTextures" << YAML::BeginSeq;
		for (const auto& [shaderName, editorName] : shader->GetResources().shaderTextureDefinitions)
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(shaderName, shaderName, out);
			VT_SERIALIZE_PROPERTY(editorName, editorName, out);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path);
		fout << out.c_str();
		fout.close();
	}

	void ShaderImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{
	}

	bool ShaderImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}

	bool MaterialImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Material>();
		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", path, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node rootMaterialNode = root["Material"];

		std::string nameString;
		VT_DESERIALIZE_PROPERTY(name, nameString, rootMaterialNode, std::string("Null"));

		std::unordered_map<uint32_t, Ref<SubMaterial>> materials;

		YAML::Node materialsNode = rootMaterialNode["materials"];
		for (const auto& materialNode : materialsNode)
		{
			std::string materialNameString;
			VT_DESERIALIZE_PROPERTY(material, materialNameString, materialNode, std::string("Null"));

			uint32_t materialIndex;
			VT_DESERIALIZE_PROPERTY(index, materialIndex, materialNode, 0);

			std::string shaderNameString;
			VT_DESERIALIZE_PROPERTY(shader, shaderNameString, materialNode, std::string("Illum"));

			uint32_t materialFlags;
			VT_DESERIALIZE_PROPERTY(flags, materialFlags, materialNode, (uint32_t)(MaterialFlag::Deferred | MaterialFlag::CastAO | MaterialFlag::CastShadows));

			bool isPermutation;
			VT_DESERIALIZE_PROPERTY(isPermutation, isPermutation, materialNode, false);

			std::unordered_map<std::string, Ref<Texture2D>> textures;

			YAML::Node texturesNode = materialNode["textures"];
			for (const auto& textureNode : texturesNode)
			{
				std::string shaderName;
				VT_DESERIALIZE_PROPERTY(binding, shaderName, textureNode, std::string("Empty"));

				AssetHandle textureHandle;
				VT_DESERIALIZE_PROPERTY(handle, textureHandle, textureNode, uint64_t(0));

				Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);
				if (texture)
				{
					textures.emplace(shaderName, texture);
				}
				else
				{
					textures.emplace(shaderName, nullptr);
				}
			}

			Ref<Shader> shader = ShaderRegistry::GetShader(shaderNameString);
			if (!shader || !shader->IsValid())
			{
				shader = Renderer::GetDefaultData().defaultShader;
				VT_CORE_ERROR("Shader {0} not found or invalid! Falling back to default!", shaderNameString);
			}

			Ref<SubMaterial> material = SubMaterial::Create(materialNameString, materialIndex, shader);

			material->SetFlags((MaterialFlag)materialFlags);
			if (!material->HasFlag(MaterialFlag::Opaque) && !material->HasFlag(MaterialFlag::Transparent) && !material->HasFlag(MaterialFlag::Deferred))
			{
				material->SetFlag(MaterialFlag::Deferred, true);
			}

			if (shaderNameString == "Illum" && material->HasFlag(MaterialFlag::Opaque))
			{
				material->SetFlag(MaterialFlag::Deferred, true);
				material->SetFlag(MaterialFlag::Opaque, false);
			}

			// Get pipeline properties
			{
				Topology topology;
				CullMode cullMode;
				FillMode triangleFillMode;
				DepthMode depthMode;

				VT_DESERIALIZE_PROPERTY(topology, *(uint32_t*)&topology, materialNode, (uint32_t)Topology::TriangleList);
				VT_DESERIALIZE_PROPERTY(cullMode, *(uint32_t*)&cullMode, materialNode, (uint32_t)CullMode::Back);
				VT_DESERIALIZE_PROPERTY(triangleFillMode, *(uint32_t*)&triangleFillMode, materialNode, (uint32_t)FillMode::Solid);
				VT_DESERIALIZE_PROPERTY(depthMode, *(uint32_t*)&depthMode, materialNode, (uint32_t)DepthMode::ReadWrite);

				material->myTopology = topology;
				material->myCullMode = cullMode;
				material->myTriangleFillMode = triangleFillMode;
				material->myDepthMode = depthMode;

				material->InvalidatePipeline(shader);
			}


			auto materialDataNode = materialNode["data"];
			if (materialDataNode)
			{
				VT_DESERIALIZE_PROPERTY(color, material->myMaterialData.color, materialDataNode, gem::vec4(1.f));
				VT_DESERIALIZE_PROPERTY(emissiveColor, material->myMaterialData.emissiveColor, materialDataNode, gem::vec3(1.f));
				VT_DESERIALIZE_PROPERTY(emissiveStrength, material->myMaterialData.emissiveStrength, materialDataNode, 1.f);
				VT_DESERIALIZE_PROPERTY(roughness, material->myMaterialData.roughness, materialDataNode, 0.5f);
				VT_DESERIALIZE_PROPERTY(metalness, material->myMaterialData.metalness, materialDataNode, 0.f);
				VT_DESERIALIZE_PROPERTY(normalStrength, material->myMaterialData.normalStrength, materialDataNode, 0.f);
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

			YAML::Node specializationDataNode = materialNode["specializationData"];
			if (specializationDataNode && material->GetMaterialSpecializationData().IsValid())
			{
				auto& materialData = material->GetMaterialSpecializationData();

				for (const auto& memberNode : specializationDataNode["members"])
				{
					std::string memberName;
					VT_DESERIALIZE_PROPERTY(name, memberName, memberNode, std::string(""));

					ShaderUniformType type;
					VT_DESERIALIZE_PROPERTY(type, type, memberNode, ShaderUniformType::Bool);

					auto it = std::find_if(materialData.GetMembers().begin(), materialData.GetMembers().end(), [&](const auto& value)
					{
						return value.first == memberName && value.second.type == type;
					});

					if (it != materialData.GetMembers().end())
					{
						switch (type)
						{
							case Volt::ShaderUniformType::Bool: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<bool>(memberName), memberNode, false); break;
							case Volt::ShaderUniformType::UInt: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<uint32_t>(memberName), memberNode, 0u); break;
							case Volt::ShaderUniformType::UInt2: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2ui>(memberName), memberNode, gem::vec2ui{ 0 }); break;
							case Volt::ShaderUniformType::UInt3: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3ui>(memberName), memberNode, gem::vec3ui{ 0 }); break;
							case Volt::ShaderUniformType::UInt4: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4ui>(memberName), memberNode, gem::vec4ui{ 0 }); break;

							case Volt::ShaderUniformType::Int: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<int32_t>(memberName), memberNode, 0); break;
							case Volt::ShaderUniformType::Int2: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2i>(memberName), memberNode, gem::vec2i{ 0 }); break;
							case Volt::ShaderUniformType::Int3: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3i>(memberName), memberNode, gem::vec3i{ 0 }); break;
							case Volt::ShaderUniformType::Int4: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4i>(memberName), memberNode, gem::vec4i{ 0 }); break;

							case Volt::ShaderUniformType::Float: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<float>(memberName), memberNode, 0.f); break;
							case Volt::ShaderUniformType::Float2: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2>(memberName), memberNode, gem::vec2{ 0.f }); break;
							case Volt::ShaderUniformType::Float3: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3>(memberName), memberNode, gem::vec3{ 0.f }); break;
							case Volt::ShaderUniformType::Float4: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4>(memberName), memberNode, gem::vec4{ 0.f }); break;
						}
					}
				}
			}

			YAML::Node pipelineGenerationNode = materialNode["pipelineGenerationData"];
			if (isPermutation && pipelineGenerationNode && !material->GetPipelineGenerationDatas().empty())
			{
				std::map<ShaderStage, ShaderDataBuffer>& generationData = material->GetPipelineGenerationDatas();

				for (const auto& generationDataNode : pipelineGenerationNode)
				{
					ShaderStage stage;
					VT_DESERIALIZE_PROPERTY(stage, stage, generationDataNode, ShaderStage::None);

					if (!generationData.contains(stage))
					{
						continue;
					}

					for (const auto& memberNode : generationDataNode["members"])
					{
						std::string memberName;
						VT_DESERIALIZE_PROPERTY(name, memberName, memberNode, std::string(""));

						ShaderUniformType type;
						VT_DESERIALIZE_PROPERTY(type, type, memberNode, ShaderUniformType::Bool);

						auto it = std::find_if(generationData.at(stage).GetMembers().begin(), generationData.at(stage).GetMembers().end(), [&](const auto& value)
						{
							return value.first == memberName && value.second.type == type;
						});

						if (it != generationData.at(stage).GetMembers().end())
						{
							switch (type)
							{
								case Volt::ShaderUniformType::Bool: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<bool>(memberName), memberNode, false); break;
								case Volt::ShaderUniformType::UInt: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<uint32_t>(memberName), memberNode, 0u); break;
								case Volt::ShaderUniformType::UInt2: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<gem::vec2ui>(memberName), memberNode, gem::vec2ui{ 0 }); break;
								case Volt::ShaderUniformType::UInt3: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<gem::vec3ui>(memberName), memberNode, gem::vec3ui{ 0 }); break;
								case Volt::ShaderUniformType::UInt4: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<gem::vec4ui>(memberName), memberNode, gem::vec4ui{ 0 }); break;

								case Volt::ShaderUniformType::Int: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<int32_t>(memberName), memberNode, 0); break;
								case Volt::ShaderUniformType::Int2: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<gem::vec2i>(memberName), memberNode, gem::vec2i{ 0 }); break;
								case Volt::ShaderUniformType::Int3: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<gem::vec3i>(memberName), memberNode, gem::vec3i{ 0 }); break;
								case Volt::ShaderUniformType::Int4: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<gem::vec4i>(memberName), memberNode, gem::vec4i{ 0 }); break;

								case Volt::ShaderUniformType::Float: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<float>(memberName), memberNode, 0.f); break;
								case Volt::ShaderUniformType::Float2: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<gem::vec2>(memberName), memberNode, gem::vec2{ 0.f }); break;
								case Volt::ShaderUniformType::Float3: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<gem::vec3>(memberName), memberNode, gem::vec3{ 0.f }); break;
								case Volt::ShaderUniformType::Float4: VT_DESERIALIZE_PROPERTY(data, generationData.at(stage).GetValue<gem::vec4>(memberName), memberNode, gem::vec4{ 0.f }); break;
							}
						}
					}
				}

				material->RecompilePermutation();
			}

			Renderer::UpdateMaterial(material.get());

			materials.emplace(materialIndex, material);
		}

		Ref<Material> material = std::reinterpret_pointer_cast<Material>(asset);
		material->myName = nameString;
		material->mySubMaterials = materials;
		material->path = path;

		return true;
	}

	void MaterialImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<Material> material = std::reinterpret_pointer_cast<Material>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(name, material->myName, out);
			{
				out << YAML::Key << "materials" << YAML::BeginSeq;
				for (const auto& [index, subMaterial] : material->mySubMaterials)
				{
					out << YAML::BeginMap;
					VT_SERIALIZE_PROPERTY(material, subMaterial->myName, out);
					VT_SERIALIZE_PROPERTY(index, index, out);
					VT_SERIALIZE_PROPERTY(shader, subMaterial->myPipeline->GetSpecification().shader->GetName(), out);
					VT_SERIALIZE_PROPERTY(isPermutation, subMaterial->GetPipeline()->IsPermutation(), out);
					VT_SERIALIZE_PROPERTY(flags, (uint32_t)subMaterial->myMaterialFlags, out);
					VT_SERIALIZE_PROPERTY(topology, (uint32_t)subMaterial->myTopology, out);
					VT_SERIALIZE_PROPERTY(cullMode, (uint32_t)subMaterial->myCullMode, out);
					VT_SERIALIZE_PROPERTY(triangleFillMode, (uint32_t)subMaterial->myTriangleFillMode, out);
					VT_SERIALIZE_PROPERTY(depthMode, (uint32_t)subMaterial->myDepthMode, out);

					out << YAML::Key << "data" << YAML::Value;
					out << YAML::BeginMap;
					{
						VT_SERIALIZE_PROPERTY(color, subMaterial->myMaterialData.color, out);
						VT_SERIALIZE_PROPERTY(emissiveColor, subMaterial->myMaterialData.emissiveColor, out);
						VT_SERIALIZE_PROPERTY(emissiveStrength, subMaterial->myMaterialData.emissiveStrength, out);
						VT_SERIALIZE_PROPERTY(roughness, subMaterial->myMaterialData.roughness, out);
						VT_SERIALIZE_PROPERTY(metalness, subMaterial->myMaterialData.metalness, out);
						VT_SERIALIZE_PROPERTY(normalStrength, subMaterial->myMaterialData.normalStrength, out);
					}
					out << YAML::EndMap;

					out << YAML::Key << "textures" << YAML::BeginSeq;
					for (const auto& [binding, texture] : subMaterial->myTextures)
					{
						out << YAML::BeginMap;
						VT_SERIALIZE_PROPERTY(binding, binding, out);

						AssetHandle textureHandle = Asset::Null();
						if (texture)
						{
							textureHandle = texture->handle;
						}

						VT_SERIALIZE_PROPERTY(handle, textureHandle, out);
						out << YAML::EndMap;
					}
					out << YAML::EndSeq;

					if (subMaterial->GetMaterialSpecializationData().IsValid())
					{
						const auto& materialData = subMaterial->GetMaterialSpecializationData();
						out << YAML::Key << "specializationData" << YAML::Value;
						out << YAML::BeginMap;
						{
							VT_SERIALIZE_PROPERTY(size, materialData.GetSize(), out);

							out << YAML::Key << "members" << YAML::BeginSeq;
							for (const auto& [memberName, memberData] : materialData.GetMembers())
							{
								out << YAML::BeginMap;
								VT_SERIALIZE_PROPERTY(name, memberName, out);
								VT_SERIALIZE_PROPERTY(type, memberData.type, out);

								switch (memberData.type)
								{
									case Volt::ShaderUniformType::Bool: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<bool>(memberName), out); break;
									case Volt::ShaderUniformType::UInt: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<uint32_t>(memberName), out); break;
									case Volt::ShaderUniformType::UInt2: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2ui>(memberName), out); break;
									case Volt::ShaderUniformType::UInt3: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3ui>(memberName), out); break;
									case Volt::ShaderUniformType::UInt4: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4ui>(memberName), out); break;

									case Volt::ShaderUniformType::Int: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<int32_t>(memberName), out); break;
									case Volt::ShaderUniformType::Int2: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2i>(memberName), out); break;
									case Volt::ShaderUniformType::Int3: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3i>(memberName), out); break;
									case Volt::ShaderUniformType::Int4: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4i>(memberName), out); break;

									case Volt::ShaderUniformType::Float: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<float>(memberName), out); break;
									case Volt::ShaderUniformType::Float2: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2>(memberName), out); break;
									case Volt::ShaderUniformType::Float3: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3>(memberName), out); break;
									case Volt::ShaderUniformType::Float4: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4>(memberName), out); break;
								}

								out << YAML::EndMap;
							}
							out << YAML::EndSeq;
						}
						out << YAML::EndMap;
					}

					if (!subMaterial->GetPipelineGenerationDatas().empty())
					{
						out << YAML::Key << "pipelineGenerationData" << YAML::BeginSeq;
						{
							for (const auto& [stage, generationData] : subMaterial->GetPipelineGenerationDatas())
							{
								out << YAML::BeginMap;
								VT_SERIALIZE_PROPERTY(stage, stage, out);
								VT_SERIALIZE_PROPERTY(size, generationData.GetSize(), out);

								out << YAML::Key << "members" << YAML::BeginSeq;
								for (const auto& [memberName, memberData] : generationData.GetMembers())
								{
									out << YAML::BeginMap;
									VT_SERIALIZE_PROPERTY(name, memberName, out);
									VT_SERIALIZE_PROPERTY(type, memberData.type, out);

									switch (memberData.type)
									{
										case Volt::ShaderUniformType::Bool: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<bool>(memberName), out); break;
										case Volt::ShaderUniformType::UInt: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<uint32_t>(memberName), out); break;
										case Volt::ShaderUniformType::UInt2: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<gem::vec2ui>(memberName), out); break;
										case Volt::ShaderUniformType::UInt3: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<gem::vec3ui>(memberName), out); break;
										case Volt::ShaderUniformType::UInt4: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<gem::vec4ui>(memberName), out); break;

										case Volt::ShaderUniformType::Int: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<int32_t>(memberName), out); break;
										case Volt::ShaderUniformType::Int2: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<gem::vec2i>(memberName), out); break;
										case Volt::ShaderUniformType::Int3: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<gem::vec3i>(memberName), out); break;
										case Volt::ShaderUniformType::Int4: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<gem::vec4i>(memberName), out); break;

										case Volt::ShaderUniformType::Float: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<float>(memberName), out); break;
										case Volt::ShaderUniformType::Float2: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<gem::vec2>(memberName), out); break;
										case Volt::ShaderUniformType::Float3: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<gem::vec3>(memberName), out); break;
										case Volt::ShaderUniformType::Float4: VT_SERIALIZE_PROPERTY(data, generationData.GetValue<gem::vec4>(memberName), out); break;
									}

									out << YAML::EndMap;
								}
								out << YAML::EndSeq;
								out << YAML::EndMap;
							}
						}
						out << YAML::EndSeq;
					}
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path);
		fout << out.c_str();
		fout.close();
	}

	void MaterialImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{
	}

	bool MaterialImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}

	bool FontImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Font>();
		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		asset = CreateRef<Font>(filePath);
		asset->path = path;
		return true;
	}

	void FontImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{
	}

	bool FontImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}

	bool PhysicsMaterialImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<PhysicsMaterial>();
		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		Ref<PhysicsMaterial> physicsMat = std::reinterpret_pointer_cast<PhysicsMaterial>(asset);

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", path, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node materialNode = root["PhysicsMaterial"];

		VT_DESERIALIZE_PROPERTY(staticFriction, physicsMat->staticFriction, materialNode, 0.1f);
		VT_DESERIALIZE_PROPERTY(dynamicFriction, physicsMat->dynamicFriction, materialNode, 0.1f);
		VT_DESERIALIZE_PROPERTY(bounciness, physicsMat->bounciness, materialNode, 0.1f);

		physicsMat->path = path;

		return false;
	}

	void PhysicsMaterialImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<PhysicsMaterial> material = std::reinterpret_pointer_cast<PhysicsMaterial>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "PhysicsMaterial" << YAML::Value;
		{
			VT_SERIALIZE_PROPERTY(staticFriction, material->staticFriction, out);
			VT_SERIALIZE_PROPERTY(dynamicFriction, material->dynamicFriction, out);
			VT_SERIALIZE_PROPERTY(bounciness, material->bounciness, out);
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path);
		fout << out.c_str();
		fout.close();
	}

	void PhysicsMaterialImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{
	}

	bool PhysicsMaterialImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}

	bool VideoImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Video>();
		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		asset = CreateRef<Video>(filePath);
		asset->path = path;
		return true;
	}

	void VideoImporter::Save(const Ref<Asset>&) const
	{
	}
	void VideoImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{
	}
	bool VideoImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}

	bool RenderPipelineImporter::Load(const std::filesystem::path& inPath, Ref<Asset>& asset) const
	{
		asset = CreateRef<RenderPipeline>();
		Ref<RenderPipeline> renderPipeline = std::reinterpret_pointer_cast<RenderPipeline>(asset);

		if (!std::filesystem::exists(inPath)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", inPath.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(inPath);
		if (!file.is_open()) [[unlikely]]
		{
			VT_CORE_ERROR("Failed to open file: {0}!", inPath.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", inPath, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		auto pipelineNode = root["RenderPipeline"];

		RenderPipelineSpecification specification{};

		std::string shaderName;
		VT_DESERIALIZE_PROPERTY(Shader, shaderName, pipelineNode, std::string(""));

		Ref<Shader> shader;
		if (!ShaderRegistry::IsShaderRegistered(shaderName))
		{
			VT_CORE_ERROR("Unable to find shader {0}! Falling back to default!", shaderName.c_str());
			shader = Renderer::GetDefaultData().defaultShader;
		}
		else
		{
			shader = ShaderRegistry::GetShader(shaderName);
		}

		uint32_t topology = 0;
		uint32_t cullMode = 0;
		uint32_t fillMode = 0;
		uint32_t depthMode = 0;

		VT_DESERIALIZE_PROPERTY(topology, topology, pipelineNode, 0);
		VT_DESERIALIZE_PROPERTY(cullMode, cullMode, pipelineNode, 0);
		VT_DESERIALIZE_PROPERTY(fillMode, fillMode, pipelineNode, 0);
		VT_DESERIALIZE_PROPERTY(depthMode, depthMode, pipelineNode, 0);

		specification.topology = (Topology)topology;
		specification.cullMode = (CullMode)cullMode;
		specification.fillMode = (FillMode)fillMode;
		specification.depthMode = (DepthMode)depthMode;
		specification.shader = shader;
		specification.vertexLayout = Vertex::GetVertexLayout();

		VT_DESERIALIZE_PROPERTY(lineWidth, specification.lineWidth, pipelineNode, 1.f);
		VT_DESERIALIZE_PROPERTY(tessellationControlPoints, specification.tessellationControlPoints, pipelineNode, 4);
		VT_DESERIALIZE_PROPERTY(name, specification.name, pipelineNode, std::string("Null"));

		asset = RenderPipeline::Create(specification);
		return true;
	}

	void RenderPipelineImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<RenderPipeline> renderPipeline = std::reinterpret_pointer_cast<RenderPipeline>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "RenderPipeline" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Shader" << YAML::Value << renderPipeline->GetSpecification().shader->GetName();

			VT_SERIALIZE_PROPERTY(topology, (uint32_t)renderPipeline->mySpecification.topology, out);
			VT_SERIALIZE_PROPERTY(cullMode, (uint32_t)renderPipeline->mySpecification.cullMode, out);
			VT_SERIALIZE_PROPERTY(fillMode, (uint32_t)renderPipeline->mySpecification.fillMode, out);
			VT_SERIALIZE_PROPERTY(depthMode, (uint32_t)renderPipeline->mySpecification.depthMode, out);

			VT_SERIALIZE_PROPERTY(lineWidth, renderPipeline->mySpecification.lineWidth, out);
			VT_SERIALIZE_PROPERTY(tessellationControlPoints, renderPipeline->mySpecification.tessellationControlPoints, out);
			VT_SERIALIZE_PROPERTY(name, renderPipeline->mySpecification.name, out);

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path);
		fout << out.c_str();
		fout.close();
	}

	void RenderPipelineImporter::SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const
	{
	}

	bool RenderPipelineImporter::LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const
	{
		return false;
	}

	bool BlendSpaceImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<BlendSpace>();
		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", path, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node rootBlendSpaceNode = root["BlendSpace"];

		uint32_t dimension = 0;
		gem::vec2 horizontalValues;
		gem::vec2 verticalValues;
		std::vector<std::pair<gem::vec2, AssetHandle>> animations;

		VT_DESERIALIZE_PROPERTY(dimension, dimension, rootBlendSpaceNode, 0u);
		VT_DESERIALIZE_PROPERTY(horizontalValues, horizontalValues, rootBlendSpaceNode, gem::vec2(-1.f, 1.f));
		VT_DESERIALIZE_PROPERTY(verticalValues, verticalValues, rootBlendSpaceNode, gem::vec2(-1.f, 1.f));

		for (const auto& animNode : rootBlendSpaceNode["Animations"])
		{
			auto& [value, anim] = animations.emplace_back();
			VT_DESERIALIZE_PROPERTY(animation, anim, animNode, AssetHandle(0));
			VT_DESERIALIZE_PROPERTY(value, value, animNode, gem::vec2(0.f));
		}

		Ref<BlendSpace> blendSpace = std::reinterpret_pointer_cast<BlendSpace>(asset);
		blendSpace->myAnimations = animations;
		blendSpace->myDimension = (BlendSpaceDimension)dimension;
		blendSpace->myHorizontalValues = horizontalValues;
		blendSpace->myVerticalValues = verticalValues;

		return true;
	}

	void BlendSpaceImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<BlendSpace> blendSpace = std::reinterpret_pointer_cast<BlendSpace>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "BlendSpace" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(dimension, (uint32_t)blendSpace->myDimension, out);
			VT_SERIALIZE_PROPERTY(horizontalValues, blendSpace->myHorizontalValues, out);
			VT_SERIALIZE_PROPERTY(verticalValues, blendSpace->myVerticalValues, out);

			out << YAML::Key << "Animations" << YAML::BeginSeq;
			for (const auto& anim : blendSpace->myAnimations)
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(animation, anim.second, out);
				VT_SERIALIZE_PROPERTY(value, anim.first, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path);
		fout << out.c_str();
		fout.close();
	}

	void BlendSpaceImporter::SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const
	{
	}

	bool BlendSpaceImporter::LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const
	{
		return false;
	}

	bool PostProcessingStackImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<PostProcessingStack>();
		Ref<PostProcessingStack> postStack = std::reinterpret_pointer_cast<PostProcessingStack>(asset);

		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", path, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node stackRoot = root["PostProcessingStack"];

		for (const auto& effectNode : stackRoot["Effects"])
		{
			PostProcessingEffect& effect = postStack->myPostProcessingStack.emplace_back();
			VT_DESERIALIZE_PROPERTY(materialHandle, effect.materialHandle, effectNode, AssetHandle(0));
		}

		return true;
	}

	void PostProcessingStackImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<PostProcessingStack> postStack = std::reinterpret_pointer_cast<PostProcessingStack>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "PostProcessingStack" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Effects" << YAML::BeginSeq;
			for (const auto& effect : postStack->myPostProcessingStack)
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(materialHandle, effect.materialHandle, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path);
		fout << out.c_str();
		fout.close();
	}

	void PostProcessingStackImporter::SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const
	{
	}

	bool PostProcessingStackImporter::LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const
	{
		return false;
	}

	bool PostProcessingMaterialImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<PostProcessingMaterial>();

		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", path, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node rootMaterialNode = root["PostProcessingMaterial"];

		std::string shaderName;
		VT_DESERIALIZE_PROPERTY(shader, shaderName, rootMaterialNode, std::string(""));

		if (shaderName.empty())
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		Ref<Shader> shader;
		shader = ShaderRegistry::GetShader(shaderName);
		if (!shader)
		{
			shader = Renderer::GetDefaultData().defaultPostProcessingShader;
		}

		asset = CreateRef<PostProcessingMaterial>(shader);
		Ref<PostProcessingMaterial> postMat = std::reinterpret_pointer_cast<PostProcessingMaterial>(asset);

		YAML::Node specializationDataNode = rootMaterialNode["specializationData"];
		if (specializationDataNode && postMat->myMaterialData.IsValid())
		{
			auto& materialData = postMat->myMaterialData;

			for (const auto& memberNode : specializationDataNode["members"])
			{
				std::string memberName;
				VT_DESERIALIZE_PROPERTY(name, memberName, memberNode, std::string(""));

				ShaderUniformType type;
				VT_DESERIALIZE_PROPERTY(type, type, memberNode, ShaderUniformType::Bool);

				auto it = std::find_if(materialData.GetMembers().begin(), materialData.GetMembers().end(), [&](const auto& value)
				{
					return value.first == memberName && value.second.type == type;
				});

				if (it != materialData.GetMembers().end())
				{
					switch (type)
					{
						case Volt::ShaderUniformType::Bool: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<bool>(memberName), memberNode, false); break;
						case Volt::ShaderUniformType::UInt: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<uint32_t>(memberName), memberNode, 0u); break;
						case Volt::ShaderUniformType::UInt2: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2ui>(memberName), memberNode, gem::vec2ui{ 0 }); break;
						case Volt::ShaderUniformType::UInt3: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3ui>(memberName), memberNode, gem::vec3ui{ 0 }); break;
						case Volt::ShaderUniformType::UInt4: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4ui>(memberName), memberNode, gem::vec4ui{ 0 }); break;

						case Volt::ShaderUniformType::Int: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<int32_t>(memberName), memberNode, 0); break;
						case Volt::ShaderUniformType::Int2: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2i>(memberName), memberNode, gem::vec2i{ 0 }); break;
						case Volt::ShaderUniformType::Int3: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3i>(memberName), memberNode, gem::vec3i{ 0 }); break;
						case Volt::ShaderUniformType::Int4: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4i>(memberName), memberNode, gem::vec4i{ 0 }); break;

						case Volt::ShaderUniformType::Float: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<float>(memberName), memberNode, 0.f); break;
						case Volt::ShaderUniformType::Float2: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2>(memberName), memberNode, gem::vec2{ 0.f }); break;
						case Volt::ShaderUniformType::Float3: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3>(memberName), memberNode, gem::vec3{ 0.f }); break;
						case Volt::ShaderUniformType::Float4: VT_DESERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4>(memberName), memberNode, gem::vec4{ 0.f }); break;
					}
				}
			}
		}

		postMat->path = path;
		return true;
	}

	void PostProcessingMaterialImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<PostProcessingMaterial> material = std::reinterpret_pointer_cast<PostProcessingMaterial>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "PostProcessingMaterial" << YAML::Value;
		{
			out << YAML::BeginMap;
			{
				VT_SERIALIZE_PROPERTY(shader, material->myPipeline->GetShader()->GetName(), out);

				if (material->myMaterialData.IsValid())
				{
					const auto& materialData = material->myMaterialData;
					out << YAML::Key << "specializationData" << YAML::Value;
					out << YAML::BeginMap;
					{
						VT_SERIALIZE_PROPERTY(size, materialData.GetSize(), out);

						out << YAML::Key << "members" << YAML::BeginSeq;
						for (const auto& [memberName, memberData] : materialData.GetMembers())
						{
							out << YAML::BeginMap;
							VT_SERIALIZE_PROPERTY(name, memberName, out);
							VT_SERIALIZE_PROPERTY(type, memberData.type, out);

							switch (memberData.type)
							{
								case Volt::ShaderUniformType::Bool: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<bool>(memberName), out); break;
								case Volt::ShaderUniformType::UInt: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<uint32_t>(memberName), out); break;
								case Volt::ShaderUniformType::UInt2: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2ui>(memberName), out); break;
								case Volt::ShaderUniformType::UInt3: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3ui>(memberName), out); break;
								case Volt::ShaderUniformType::UInt4: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4ui>(memberName), out); break;

								case Volt::ShaderUniformType::Int: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<int32_t>(memberName), out); break;
								case Volt::ShaderUniformType::Int2: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2i>(memberName), out); break;
								case Volt::ShaderUniformType::Int3: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3i>(memberName), out); break;
								case Volt::ShaderUniformType::Int4: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4i>(memberName), out); break;

								case Volt::ShaderUniformType::Float: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<float>(memberName), out); break;
								case Volt::ShaderUniformType::Float2: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec2>(memberName), out); break;
								case Volt::ShaderUniformType::Float3: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec3>(memberName), out); break;
								case Volt::ShaderUniformType::Float4: VT_SERIALIZE_PROPERTY(data, materialData.GetValue<gem::vec4>(memberName), out); break;
							}

							out << YAML::EndMap;
						}
						out << YAML::EndSeq;
					}
					out << YAML::EndMap;
				}
				out << YAML::EndMap;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path);
		fout << out.c_str();
		fout.close();
	}

	void PostProcessingMaterialImporter::SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const
	{
	}

	bool PostProcessingMaterialImporter::LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const
	{
		return false;
	}
}
