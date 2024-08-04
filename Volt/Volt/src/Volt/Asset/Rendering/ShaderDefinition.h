#pragma once

#include "Volt/Asset/Asset.h"
#include <RHIModule/Shader/Shader.h>

namespace Volt
{
	class ShaderDefinition : public Asset
	{
	public:
		~ShaderDefinition() override = default;

		static AssetType GetStaticType() { return AssetType::ShaderDefinition; }
		inline AssetType GetType() override { return GetStaticType(); }
		inline uint32_t GetVersion() const override { return 1; }

		VT_NODISCARD VT_INLINE const Vector<RHI::ShaderSourceEntry>& GetSourceEntries() const { return m_sourceEntries; }
		VT_NODISCARD VT_INLINE const Vector<std::string>& GetPermutations() { return m_permutaionValues; }
		VT_NODISCARD VT_INLINE std::string_view GetName() const { return m_name; }
		VT_NODISCARD VT_INLINE const bool IsInternal() const { return m_isInternal; }

	private:
		friend class ShaderDefinitionImporter;
		friend class ShaderDefinitionSerializer;

		Vector<std::string> m_permutaionValues;
		Vector<RHI::ShaderSourceEntry> m_sourceEntries;

		std::string m_name;
		bool m_isInternal = false;
	};
}
