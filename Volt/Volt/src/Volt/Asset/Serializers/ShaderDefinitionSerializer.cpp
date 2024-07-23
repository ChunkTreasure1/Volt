#include "vtpch.h"
#include "ShaderDefinitionSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Rendering/ShaderDefinition.h"
#include "Volt/Utility/StringUtility.h"

#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>
#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>

namespace Volt
{
	namespace Utility
	{
		inline RHI::ShaderStage GetShaderStageFromString(const std::string& string)
		{
			std::string lowerString = ::Utility::ToLower(string);

			if (lowerString == "vs")
			{
				return RHI::ShaderStage::Vertex;
			}
			else if (lowerString == "ps" || lowerString == "fs")
			{
				return RHI::ShaderStage::Pixel;
			}
			else if (lowerString == "cs")
			{
				return RHI::ShaderStage::Compute;
			}
			else if (lowerString == "gs")
			{
				return RHI::ShaderStage::Geometry;
			}
			else if (lowerString == "hs")
			{
				return RHI::ShaderStage::Hull;
			}
			else if (lowerString == "ds")
			{
				return RHI::ShaderStage::Domain;
			}
			else if (lowerString == "rgen")
			{
				return RHI::ShaderStage::RayGen;
			}
			else if (lowerString == "rmiss")
			{
				return RHI::ShaderStage::Miss;
			}
			else if (lowerString == "rchit")
			{
				return RHI::ShaderStage::ClosestHit;
			}
			else if (lowerString == "rahit")
			{
				return RHI::ShaderStage::AnyHit;
			}
			else if (lowerString == "rinter")
			{
				return RHI::ShaderStage::Intersection;
			}
			else if (lowerString == "as")
			{
				return RHI::ShaderStage::Amplification;
			}
			else if (lowerString == "ms")
			{
				return RHI::ShaderStage::Mesh;
			}

			VT_ASSERT_MSG(false, "Stage is not valid!");
			return RHI::ShaderStage::Vertex;
		}

		inline constexpr std::string_view GetStringFromShaderStage(RHI::ShaderStage stage)
		{
			switch (stage)
			{
				case RHI::ShaderStage::Vertex: return "vs";
				case RHI::ShaderStage::Pixel: return "ps";
				case RHI::ShaderStage::Hull: return "hs";
				case RHI::ShaderStage::Domain: return "ds";
				case RHI::ShaderStage::Geometry: return "gs";
				case RHI::ShaderStage::Compute: return "cs";
				case RHI::ShaderStage::RayGen: return "rgen";
				case RHI::ShaderStage::AnyHit: return "rahit";
				case RHI::ShaderStage::ClosestHit: return "rchit";
				case RHI::ShaderStage::Miss: return "rmiss";
				case RHI::ShaderStage::Intersection: return "rinter";
				case RHI::ShaderStage::Amplification: return "as";
				case RHI::ShaderStage::Mesh: return "ms";
			}

			VT_ASSERT_MSG(false, "Stage is not valid!");
			return "";
		}
	}

	void ShaderDefinitionSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<ShaderDefinition> shaderDef = std::reinterpret_pointer_cast<ShaderDefinition>(asset);
		
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		YAMLFileStreamWriter yamlStreamWriter{ filePath };
		yamlStreamWriter.BeginMap();

		// Serialize AssetMetadata
		yamlStreamWriter.SetKey("assetHandle", metadata.handle);
		yamlStreamWriter.SetKey("type", (uint32_t)metadata.type);
		yamlStreamWriter.SetKey("version", shaderDef->GetVersion());
		//

		yamlStreamWriter.SetKey("name", shaderDef->GetName());
		yamlStreamWriter.SetKey("internal", shaderDef->IsInternal());

		yamlStreamWriter.BeginSequence("sources");
		for (const auto& entry : shaderDef->GetSourceEntries())
		{
			yamlStreamWriter.BeginMap();
			yamlStreamWriter.SetKey("entryPoint", entry.entryPoint);
			yamlStreamWriter.SetKey("filePath", entry.filePath);
			yamlStreamWriter.SetKey("shaderStage", std::string(Utility::GetStringFromShaderStage(entry.shaderStage)));
			yamlStreamWriter.EndMap();
		}
		yamlStreamWriter.EndSequence();
		yamlStreamWriter.EndMap();
	
		yamlStreamWriter.WriteToDisk();
	}

	bool ShaderDefinitionSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLFileStreamReader yamlStreamReader{};

		if (!yamlStreamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata{};
		serializedMetadata.handle = yamlStreamReader.ReadAtKey("assetHandle", AssetHandle(0));
		serializedMetadata.type = (AssetType)yamlStreamReader.ReadAtKey("type", 0u);
		serializedMetadata.version = yamlStreamReader.ReadAtKey("version", 1u);

		VT_ASSERT_MSG(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		const std::string name = yamlStreamReader.ReadAtKey("name", std::string("Unnamed"));
		const bool isInternal = yamlStreamReader.ReadAtKey("internal", false);

		if (!yamlStreamReader.HasKey("sources"))
		{
			VT_CORE_ERROR("No shaders defined in shader definition {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		Vector<RHI::ShaderSourceEntry> entries;
		yamlStreamReader.ForEach("sources", [&]()
		{
			auto& entry = entries.emplace_back();
			entry.entryPoint = yamlStreamReader.ReadAtKey("entryPoint", std::string("main"));
			entry.filePath = yamlStreamReader.ReadAtKey("filePath", std::filesystem::path(""));
			entry.shaderStage = Utility::GetShaderStageFromString(yamlStreamReader.ReadAtKey("shaderStage", std::string("")));
		});

		Vector<std::string> permutationValues;
		if (yamlStreamReader.HasKey("permutations"))
		{
			yamlStreamReader.ForEach("permutations", [&]()
			{
				permutationValues.emplace_back(yamlStreamReader.ReadValue<std::string>());
			});
		}

		Ref<ShaderDefinition> shaderDef = std::reinterpret_pointer_cast<ShaderDefinition>(destinationAsset);
		shaderDef->m_isInternal = isInternal;
		shaderDef->m_name = name;
		shaderDef->m_sourceEntries = entries;
		shaderDef->m_permutaionValues = permutationValues;

		return true;
	}
}
