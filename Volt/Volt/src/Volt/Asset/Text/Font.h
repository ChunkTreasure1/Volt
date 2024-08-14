#pragma once

#include "Volt/Asset/AssetTypes.h"

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetFactory.h>

namespace Volt
{
	struct MSDFData;
	class Texture2D;

	class Font : public Asset
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

		float GetStringWidth(const std::string& string, const glm::vec2& scale, float maxWidth);
		float GetStringHeight(const std::string& string, const glm::vec2& scale, float maxWidth);

		inline Ref<Texture2D> GetAtlas() const { return myAtlas; }
		inline MSDFData* GetMSDFData() const { return myMSDFData; }

		static AssetType GetStaticType() { return AssetTypes::Font; }
		AssetType GetType() override { return GetStaticType(); };
		uint32_t GetVersion() const override { return 1; }

	private:
		MSDFData* myMSDFData = nullptr;
		Ref<Texture2D> myAtlas;
	};
}
