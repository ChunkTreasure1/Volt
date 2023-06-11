#pragma once

#include "Volt/Asset/Asset.h"

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
		Font(const std::filesystem::path& aPath);
		~Font() override;

		float GetStringWidth(const std::string& string, const glm::vec2& scale, float maxWidth);
		inline Ref<Texture2D> GetAtlas() const { return myAtlas; }
		inline MSDFData* GetMSDFData() const { return myMSDFData; }

		static AssetType GetStaticType() { return AssetType::Font; }
		AssetType GetType() override { return GetStaticType(); };

	private:
		MSDFData* myMSDFData = nullptr;
		Ref<Texture2D> myAtlas;
	};
}
