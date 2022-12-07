#include "vtpch.h"
#include "AssetImporter.h"

#include "Volt/Asset//AssetManager.h"
#include "Volt/Asset/Importers/TextureImporter.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/Text/Font.h"
#include "Volt/Asset/Video/Video.h"

#include "Volt/Physics/PhysicsMaterial.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/Shader/ShaderRegistry.h"

#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <yaml-cpp/yaml.h>

namespace Volt
{
	bool TextureSourceImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Texture2D>();

		if (!std::filesystem::exists(path)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}
		auto mesh = TextureImporter::ImportTexture(path);

		if (!mesh) [[unlikely]]
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		asset = mesh;
		asset->path = path;
		return true;
	}

	void TextureSourceImporter::Save(const Ref<Asset>&) const
	{}

	void TextureSourceImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{}

	bool TextureSourceImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}

	bool ShaderImporter::Load(const std::filesystem::path& inPath, Ref<Asset>& asset) const
	{
		asset = CreateRef<Shader>();
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
		std::unordered_map<uint32_t, std::string> inputTextures; // binding -> name
		for (const auto input : inputTexturesNode)
		{
			uint32_t binding;
			VT_DESERIALIZE_PROPERTY(binding, binding, input, 0);

			std::string texName;
			VT_DESERIALIZE_PROPERTY(name, texName, input, std::string("Null"));

			inputTextures.emplace(binding, texName);
		}

		Ref<Shader> shader = Shader::Create(name, paths, false, isInternal);

		// Make sure all textures defined in definition actually exist
		{
			const auto& textureInfos = shader->GetResources().textures;
			auto& shaderInputDefinitions = const_cast<std::unordered_map<uint32_t, std::string>&>(shader->GetResources().shaderTextureDefinitions);

			for (const auto& [binding, texName] : inputTextures)
			{
				if (binding >= Shader::MinCustomTextureSlot() || binding <= Shader::MaxCustomTextureSlot())
				{
					auto it = textureInfos.find(binding);
					if (it == textureInfos.end())
					{
						VT_CORE_WARN("Shader {0} does not have a texture input with binding {1}, but this is defined in definition!", inPath.string().c_str(), binding);
					}
					else
					{
						shaderInputDefinitions.emplace(binding, texName);
					}
				}
			}
		}

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

	void ShaderImporter::Save(const Ref<Asset>&) const
	{}

	void ShaderImporter::SaveBinary(uint8_t *, const Ref<Asset>&) const
	{}

	bool ShaderImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader &, Ref<Asset>&) const
	{
		return false;
	}

	bool MaterialImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Material>();
		if (!std::filesystem::exists(path))
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(path);
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
			VT_DESERIALIZE_PROPERTY(shader, shaderNameString, materialNode, std::string("Null"));

			std::unordered_map<uint32_t, Ref<Texture2D>> textures;

			YAML::Node texturesNode = materialNode["textures"];
			for (const auto& textureNode : texturesNode)
			{
				uint32_t textureBinding;
				VT_DESERIALIZE_PROPERTY(binding, textureBinding, textureNode, 0);

				AssetHandle textureHandle;
				VT_DESERIALIZE_PROPERTY(handle, textureHandle, textureNode, uint64_t(0));

				Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);
				if (textureHandle == Asset::Null() || !texture || !texture->IsValid())
				{
					if (textureBinding == 1)
					{
						texture = Renderer::GetDefaultData().emptyNormal;
					}
					else
					{
						texture = Renderer::GetDefaultData().whiteTexture;
					}
				}

				textures.emplace(textureBinding, texture);
			}

			Ref<Shader> shader = ShaderRegistry::Get(shaderNameString);
			if (!shader || !shader->IsValid())
			{
				shader = Renderer::GetDefaultData().defaultShader;
				VT_CORE_ERROR("Shader {0} not found or invalid! Falling back to default!", shaderNameString);
			}

			Ref<SubMaterial> material = SubMaterial::Create(materialNameString, materialIndex, shader);
			for (const auto& [binding, texture] : textures)
			{
				const auto& textureDefinitions = shader->GetResources().shaderTextureDefinitions;

				if (textureDefinitions.find(binding) != textureDefinitions.end())
				{
					material->SetTexture(binding, texture);
				}
			}

			YAML::Node bufferNode = materialNode["buffer"];
			if (bufferNode && material->myShaderResources.materialBuffer.exists)
			{
				auto& materialBuffer = material->myShaderResources.materialBuffer;

				for (const auto& paramNode : bufferNode["params"])
				{
					std::string paramNameString;
					VT_DESERIALIZE_PROPERTY(name, paramNameString, paramNode, std::string());

					ElementType paramType;
					VT_DESERIALIZE_PROPERTY(type, paramType, paramNode, ElementType::Bool);

					auto it = std::find_if(materialBuffer.parameters.begin(), materialBuffer.parameters.end(), [paramNameString, paramType](const std::pair<std::string, Shader::MaterialBuffer::Parameter>& value)
						{
							return value.first == paramNameString && value.second.type == paramType;
						});

					if (it != materialBuffer.parameters.end())
					{
						switch (paramType)
						{
							case ElementType::Bool: VT_DESERIALIZE_PROPERTY(data, *(bool*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, false); break;
							case ElementType::Int: VT_DESERIALIZE_PROPERTY(data, *(int32_t*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, 0); break;

							case ElementType::UInt: VT_DESERIALIZE_PROPERTY(data, *(uint32_t*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, 0); break;
							case ElementType::UInt2: VT_DESERIALIZE_PROPERTY(data, *(gem::vec2ui*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, gem::vec2ui{ 0 }); break;
							case ElementType::UInt3: VT_DESERIALIZE_PROPERTY(data, *(gem::vec3ui*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, gem::vec3ui{ 0 }); break;
							case ElementType::UInt4: VT_DESERIALIZE_PROPERTY(data, *(gem::vec4ui*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, gem::vec4ui{ 0 }); break;

							case ElementType::Float: VT_DESERIALIZE_PROPERTY(data, *(float*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, 0.f); break;
							case ElementType::Float2: VT_DESERIALIZE_PROPERTY(data, *(gem::vec2*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, gem::vec2{ 0.f }); break;
							case ElementType::Float3: VT_DESERIALIZE_PROPERTY(data, *(gem::vec3*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, gem::vec3{ 0.f }); break;
							case ElementType::Float4: VT_DESERIALIZE_PROPERTY(data, *(gem::vec4*)&materialBuffer.data[materialBuffer.parameters[paramNameString].offset], paramNode, gem::vec4{ 0.f }); break;
						}
					}
				}

				material->UpdateBuffer(true);
			}

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
					VT_SERIALIZE_PROPERTY(shader, subMaterial->myShader->GetName(), out);

					out << YAML::Key << "textures" << YAML::BeginSeq;
					for (const auto& [binding, texture] : subMaterial->myTextures)
					{
						out << YAML::BeginMap;
						VT_SERIALIZE_PROPERTY(binding, binding, out);
						VT_SERIALIZE_PROPERTY(handle, texture->handle, out);
						out << YAML::EndMap;
					}
					out << YAML::EndSeq;

					if (subMaterial->GetResources().materialBuffer.exists)
					{
						const auto& materialBuffer = subMaterial->GetResources().materialBuffer;

						out << YAML::Key << "buffer" << YAML::Value;
						out << YAML::BeginMap;
						{
							VT_SERIALIZE_PROPERTY(size, materialBuffer.size, out);

							out << YAML::Key << "params" << YAML::BeginSeq;
							for (const auto& [paramName, param] : materialBuffer.parameters)
							{
								out << YAML::BeginMap;
								VT_SERIALIZE_PROPERTY(name, paramName, out);
								VT_SERIALIZE_PROPERTY(type, param.type, out);

								switch (param.type)
								{
									case ElementType::Bool: VT_SERIALIZE_PROPERTY(data, *(bool*)&materialBuffer.data[param.offset], out); break;
									case ElementType::Int: VT_SERIALIZE_PROPERTY(data, *(int32_t*)&materialBuffer.data[param.offset], out); break;

									case ElementType::UInt: VT_SERIALIZE_PROPERTY(data, *(uint32_t*)&materialBuffer.data[param.offset], out); break;
									case ElementType::UInt2: VT_SERIALIZE_PROPERTY(data, *(gem::vec2ui*)&materialBuffer.data[param.offset], out); break;
									case ElementType::UInt3: VT_SERIALIZE_PROPERTY(data, *(gem::vec3ui*)&materialBuffer.data[param.offset], out); break;
									case ElementType::UInt4: VT_SERIALIZE_PROPERTY(data, *(gem::vec4ui*)&materialBuffer.data[param.offset], out); break;

									case ElementType::Float: VT_SERIALIZE_PROPERTY(data, *(float*)&materialBuffer.data[param.offset], out); break;
									case ElementType::Float2: VT_SERIALIZE_PROPERTY(data, *(gem::vec2*)&materialBuffer.data[param.offset], out); break;
									case ElementType::Float3: VT_SERIALIZE_PROPERTY(data, *(gem::vec3*)&materialBuffer.data[param.offset], out); break;
									case ElementType::Float4: VT_SERIALIZE_PROPERTY(data, *(gem::vec4*)&materialBuffer.data[param.offset], out); break;
								}

								out << YAML::EndMap;
							}
							out << YAML::EndSeq;
						}
						out << YAML::EndMap;
					}
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(asset->path);
		fout << out.c_str();
		fout.close();
	}

	void MaterialImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{}

	bool MaterialImporter::LoadBinary(const uint8_t *, const AssetPacker::AssetHeader &, Ref<Asset>&) const
	{
		return false;
	}

	bool FontImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Font>();
		if (!std::filesystem::exists(path))
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		asset = CreateRef<Font>(path);
		asset->path = path;
		return true;
	}

	void FontImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{}

	bool FontImporter::LoadBinary(const uint8_t *, const AssetPacker::AssetHeader &, Ref<Asset>&) const
	{
		return false;
	}

	bool PhysicsMaterialImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<PhysicsMaterial>();
		if (!std::filesystem::exists(path))
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(path);
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

		std::ofstream fout(asset->path);
		fout << out.c_str();
		fout.close();
	}

	void PhysicsMaterialImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{}

	bool PhysicsMaterialImporter::LoadBinary(const uint8_t *, const AssetPacker::AssetHeader &, Ref<Asset>&) const
	{
		return false;
	}

	bool VideoImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Video>();

		if (!std::filesystem::exists(path)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		asset = CreateRef<Video>(path);
		asset->path = path;
		return true;
	}

	void VideoImporter::Save(const Ref<Asset>&) const
	{}
	void VideoImporter::SaveBinary(uint8_t * , const Ref<Asset>&) const
	{}
	bool VideoImporter::LoadBinary(const uint8_t *, const AssetPacker::AssetHeader &, Ref<Asset>&) const
	{
		return false;
	}
}