#include "vtpch.h"
#include "Material.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/MosaicNodes/TextureNodes.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/RenderingNew/RendererNew.h"

#include <Mosaic/MosaicNode.h>

#include <CoreUtilities/GUIDUtilities.h>

#include <VoltRHI/Pipelines/ComputePipeline.h>
#include <VoltRHI/Shader/Shader.h>

namespace Volt
{
	Material::Material()
	{
		m_graph = Mosaic::MosaicGraph::CreateDefaultGraph();
		m_materialGUID = GUIDUtilities::GenerateGUID();
	}

	Material::Material(Ref<RHI::ComputePipeline> computePipeline)
		: m_computePipeline(computePipeline)
	{
	}

	const std::string& Material::GetName() const
	{
		return assetName;
	}

	bool Material::HasFlag() const
	{
		return false;
	}

	void Material::SetTexture(Ref<Texture2D> texture, uint32_t index)
	{
		if (static_cast<size_t>(index) >= m_textures.size())
		{
			m_textures.resize(index + 1);
		}

		m_textures[index] = texture;
	}

	void Material::RemoveTexture(uint32_t index)
	{
		if (static_cast<size_t>(index) >= m_textures.size())
		{
			return;
		}

		m_textures.erase(m_textures.begin() + index);
	}

	void Material::Compile()
	{
		constexpr const char* REPLACE_STRING = "GENERATED_SHADER";
		constexpr const char* BASE_OUTPUT_PATH = "Generated\\Materials";
		constexpr const char* BASE_SHADER_PATH = "Engine\\Shaders\\Source\\Generated\\GenerateGBuffer_cs.hlsl";

		constexpr size_t REPLACE_STRING_SIZE = 16;
		constexpr VoltGUID TEXTURE_NODE_GUID = "{DB60F69D-EFC5-4AA4-BF5A-C89D58942D3F}"_guid;

		const auto baseShaderPath = ProjectManager::GetEngineDirectory() / BASE_SHADER_PATH;
		const std::string compilationResult = m_graph->Compile();

		// #TODO_Ivar: Temporary barrier
		if (compilationResult.empty())
		{
			return;
		}

		std::ifstream input(baseShaderPath, std::ios::in | std::ios::binary);
		VT_CORE_ASSERT(input.is_open(), "Could not open file!");

		std::string resultShader;

		input.seekg(0, std::ios::end);
		resultShader.resize(input.tellg());
		input.seekg(0, std::ios::beg);
		input.read(&resultShader[0], resultShader.size());

		input.close();

		const size_t replaceOffset = resultShader.find(REPLACE_STRING);
		resultShader.replace(replaceOffset, REPLACE_STRING_SIZE, compilationResult);

		const std::filesystem::path outShaderPath = ProjectManager::GetProjectDirectory() / BASE_OUTPUT_PATH / std::filesystem::path(assetName + "-" + m_materialGUID.ToString() + "_cs.hlsl");

		if (!std::filesystem::exists(outShaderPath.parent_path()))
		{
			std::filesystem::create_directories(outShaderPath.parent_path());
		}

		std::ofstream output(outShaderPath);
		VT_CORE_ASSERT(output.is_open(), "Could not open file!");

		output.write(resultShader.c_str(), resultShader.size());
		output.close();

		// Find all textures
		{
			m_textures.clear();
			m_textures.resize(m_graph->GetTextureCount());

			for (const auto& node : m_graph->GetUnderlyingGraph().GetNodes())
			{
				if (node.nodeData->GetGUID() == TEXTURE_NODE_GUID)
				{
					auto textureNode = std::reinterpret_pointer_cast<SampleTextureNode>(node.nodeData);
					const auto textureInfo = textureNode->GetTextureInfo();

					Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureInfo.textureHandle);
					if (!texture)
					{
						texture = RendererNew::GetDefaultResources().whiteTexture;
					}

					m_textures.at(textureInfo.textureIndex) = texture;
				}
			}
		}

		RHI::ShaderSpecification shaderSpecification;
		shaderSpecification.name = assetName;
		shaderSpecification.sourceFiles = { outShaderPath };
		shaderSpecification.forceCompile = true;

		m_computePipeline = RHI::ComputePipeline::Create(RHI::Shader::Create(shaderSpecification));
	}
}

