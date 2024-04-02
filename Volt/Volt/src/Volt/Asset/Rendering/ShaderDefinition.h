#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class ShaderDefinition : public Asset
	{
	public:
		~ShaderDefinition() override = default;

		static AssetType GetStaticType() { return AssetType::ShaderDefinition; }
		inline AssetType GetType() override { return GetStaticType(); }
		inline uint32_t GetVersion() const override { return 1; }

		[[nodiscard]] inline const std::vector<std::filesystem::path>& GetSourceFiles() { return m_sourceFiles; }
		[[nodiscard]] inline const std::vector<std::string>& GetPermutations() { return m_permutaionValues; }
		[[nodiscard]] inline std::string_view GetName() const { return m_name; }
		[[nodiscard]] inline std::string_view GetEntryPoint() const { return m_entryPoint; }
		[[nodiscard]] inline const bool IsInternal() const { return m_isInternal; }

	private:
		friend class ShaderDefinitionImporter;
		friend class ShaderDefinitionSerializer;

		std::vector<std::filesystem::path> m_sourceFiles;
		std::vector<std::string> m_permutaionValues;

		std::string m_name;
		std::string m_entryPoint;
		bool m_isInternal = false;
	};
}
