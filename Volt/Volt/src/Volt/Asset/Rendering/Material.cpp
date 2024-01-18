#include "vtpch.h"
#include "Material.h"

#include "Volt/Project/ProjectManager.h"

namespace Volt
{
	Material::Material()
	{
		m_graph = CreateScope<Mosaic::MosaicGraph>();
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
		constexpr const char* BASE_OUTPUT_PATH = "Generated/Materials";
		constexpr const char* BASE_SHADER_PATH = "Engine\\Shaders\\Source\\Generated\\GenerateGBuffer_cs.hlsl";

		constexpr size_t REPLACE_STRING_SIZE = 16;

		const auto baseShaderPath = ProjectManager::GetEngineDirectory() / BASE_SHADER_PATH;
		const std::string compilationResult = m_graph->Compile();

		std::ifstream input(baseShaderPath);
		VT_CORE_ASSERT(input.is_open(), "Could not open file!");

		std::string resultShader;

		input.seekg(0, std::ios::end);
		resultShader.resize(input.tellg());
		input.seekg(0, std::ios::beg);
		input.read(&resultShader[0], resultShader.size());

		input.close();

		const size_t replaceOffset = resultShader.find(REPLACE_STRING);
		resultShader.replace(replaceOffset, REPLACE_STRING_SIZE, compilationResult);

		const std::filesystem::path outShaderPath = BASE_OUTPUT_PATH / std::filesystem::path(assetName + "_cs.hlsl");

		std::ofstream output(outShaderPath);
		VT_CORE_ASSERT(output.is_open(), "Could not open file!");

		output.write(resultShader.c_str(), resultShader.size());
		output.close();
	}
}


