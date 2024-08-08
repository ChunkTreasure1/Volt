#include "vtpch.h"
#include "Material.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/MosaicNodes/Texture/SampleTextureNode.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Texture/Texture2D.h"

#include <Mosaic/MosaicNode.h>

#include <CoreUtilities/GUIDUtilities.h>

#include <RHIModule/Pipelines/ComputePipeline.h>
#include <RHIModule/Shader/Shader.h>

namespace Volt
{
	constexpr VoltGUID TEXTURE_NODE_GUID = "{DB60F69D-EFC5-4AA4-BF5A-C89D58942D3F}"_guid;

	Material::Material()
	{
		m_graph = Mosaic::MosaicGraph::CreateDefaultGraph();
		m_materialGUID = GUIDUtilities::GenerateGUID();
	}

	Material::Material(RefPtr<RHI::ComputePipeline> computePipeline)
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
		m_isDirty = true;
	}

	void Material::RemoveTexture(uint32_t index)
	{
		if (static_cast<size_t>(index) >= m_textures.size())
		{
			return;
		}

		m_textures.erase(m_textures.begin() + index);
		m_isDirty = true;
	}

	bool Material::ClearAndGetIsDirty()
	{
		bool state = m_isDirty;
		m_isDirty = false;
		return state;
	}

	Vector<AssetHandle> Material::GetTextureHandles() const
	{
		Vector<AssetHandle> result;

		for (const auto& node : m_graph->GetUnderlyingGraph().GetNodes())
		{
			if (node.nodeData->GetGUID() == TEXTURE_NODE_GUID)
			{
				auto textureNode = std::reinterpret_pointer_cast<MosaicNodes::SampleTextureNode>(node.nodeData);
				const auto textureInfo = textureNode->GetTextureInfo();
				result.emplace_back(textureInfo.textureHandle);
			}
		}

		return result;
	}

	void Material::OnDependencyChanged(AssetHandle dependencyHandle, AssetChangedState state)
	{
		VT_LOG(Trace, "Triggered dependency changed in material: {0}, with dependency {1}", (uint64_t)handle, (uint64_t)dependencyHandle);

		auto it = std::find_if(m_textures.begin(), m_textures.end(), [dependencyHandle](Ref<Texture2D> tex)
		{
			return tex != nullptr && tex->handle == dependencyHandle;
		});

		if (it == m_textures.end())
		{
			return;
		}

		if (state == AssetChangedState::Updated)
		{
			m_isDirty = true;
		}
		else if (state == AssetChangedState::Removed)
		{
			for (size_t i = 0; i < m_textures.size(); i++)
			{
				if (m_textures.at(i)->handle == dependencyHandle)
				{
					m_textures[i] = Renderer::GetDefaultResources().whiteTexture;
				}
			}

			m_isDirty = true;
		}
	}

	void Material::Compile()
	{
		constexpr const char* REPLACE_STRING = "GENERATED_SHADER";
		constexpr const char* BASE_OUTPUT_PATH = "Generated\\Materials";
		constexpr const char* BASE_SHADER_PATH = "Engine\\Shaders\\Source\\Generated\\GenerateGBuffer_cs.hlsl";

		constexpr size_t REPLACE_STRING_SIZE = 16;

		const auto baseShaderPath = ProjectManager::GetEngineDirectory() / BASE_SHADER_PATH;
		const std::string compilationResult = m_graph->Compile();

		// #TODO_Ivar: Temporary barrier
		if (compilationResult.empty())
		{
			return;
		}

		std::ifstream input(baseShaderPath, std::ios::in | std::ios::binary);
		VT_ASSERT_MSG(input.is_open(), "Could not open file!");

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
		VT_ASSERT_MSG(output.is_open(), "Could not open file!");

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
					auto textureNode = std::reinterpret_pointer_cast<MosaicNodes::SampleTextureNode>(node.nodeData);
					const auto textureInfo = textureNode->GetTextureInfo();

					auto& texture = m_textures.at(textureInfo.textureIndex);

					texture = AssetManager::QueueAsset<Texture2D>(textureInfo.textureHandle);
					if (!texture)
					{
						texture = Renderer::GetDefaultResources().whiteTexture;
					}
				}
			}
		}

		RHI::ShaderSpecification shaderSpecification;
		shaderSpecification.name = assetName;
		shaderSpecification.sourceEntries = { { "main", RHI::ShaderStage::Compute, outShaderPath}};
		shaderSpecification.forceCompile = true;

		m_computePipeline = RHI::ComputePipeline::Create(RHI::Shader::Create(shaderSpecification));

		m_isDirty = true;
	}
}


