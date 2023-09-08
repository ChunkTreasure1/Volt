#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class ShaderDefinition : public Asset
	{
	public:
		~ShaderDefinition() override = default;

		static AssetType GetStaticType() { return AssetType::ShaderDefinition; }
		AssetType GetType() override { return GetStaticType(); }

		[[nodiscard]] inline const std::vector<std::filesystem::path>& GetSourceFiles() { return m_sourceFiles; }
		[[nodiscard]] inline std::string_view GetName() const { return m_name; }
		[[nodiscard]] inline const bool IsInternal() const { return m_isInternal; }

	private:
		friend class ShaderDefinitionImporter;

		std::vector<std::filesystem::path> m_sourceFiles;
		std::string m_name;
		bool m_isInternal = false;
	};
}
