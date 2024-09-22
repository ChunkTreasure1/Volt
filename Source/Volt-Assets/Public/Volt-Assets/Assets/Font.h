#pragma once

#include "Volt-Assets/Config.h"
#include "Volt-Assets/AssetTypes.h"

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetFactory.h>

#include <RHIModule/Images/Image.h>
#include <RHIModule/Descriptors/ResourceHandle.h>

#include <CoreUtilities/Pointers/RefPtr.h>

#include <glm/glm.hpp>

namespace Volt
{
	namespace RHI
	{
		class Image;
	}

	struct MSDFData;

	class VTASSETS_API Font : public Asset
	{
	public:
		struct FontHeader
		{
			uint32_t width = 0;
			uint32_t height = 0;
		};

		Font() = default;
		~Font() override;

		void Initialize(const std::filesystem::path& filePath);

		VT_NODISCARD float GetStringWidth(const std::string& string, const glm::vec2& scale, float maxWidth);
		VT_NODISCARD float GetStringHeight(const std::string& string, const glm::vec2& scale, float maxWidth);

		VT_NODISCARD VT_INLINE RefPtr<RHI::Image> GetAtlas() const { return m_atlas; }
		VT_NODISCARD VT_INLINE ResourceHandle GetAtlasResourceHandle() const { return m_atlasResourceHandle; }
		VT_NODISCARD VT_INLINE MSDFData* GetMSDFData() const { return m_msdfData; }

		static AssetType GetStaticType() { return AssetTypes::Font; }
		AssetType GetType() override { return GetStaticType(); };
		uint32_t GetVersion() const override { return 1; }

	private:
		MSDFData* m_msdfData = nullptr;
		RefPtr<RHI::Image> m_atlas;
		ResourceHandle m_atlasResourceHandle;
	};
}
