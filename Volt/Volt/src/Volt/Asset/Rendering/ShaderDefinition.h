#pragma once

#include "Volt/Asset/Asset.h"
#include <VoltRHI/Shader/Shader.h>

namespace Volt
{
	class ShaderDefinition : public Asset
	{
	public:
		~ShaderDefinition() override = default;

		static AssetType GetStaticType() { return AssetType::ShaderDefinition; }
		inline AssetType GetType() override { return GetStaticType(); }
		inline uint32_t GetVersion() const override { return 1; }

		VT_NODISCARD VT_INLINE const std::vector<RHI::ShaderSourceEntry>& GetSourceEntries() const { return m_sourceEntries; }
		VT_NODISCARD VT_INLINE const std::vector<std::string>& GetPermutations() { return m_permutaionValues; }
		VT_NODISCARD VT_INLINE std::string_view GetName() const { return m_name; }
		VT_NODISCARD VT_INLINE const bool IsInternal() const { return m_isInternal; }

	private:
		friend class ShaderDefinitionImporter;
		friend class ShaderDefinitionSerializer;

		std::vector<std::string> m_permutaionValues;
		std::vector<RHI::ShaderSourceEntry> m_sourceEntries;

		std::string m_name;
		bool m_isInternal = false;
	};
}
