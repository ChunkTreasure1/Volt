#pragma once

#include "Volt/Asset/Importers/AssetImporter.h"

namespace Volt
{
	class AnimationImporter : public AssetImporter
	{
	public:
		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	
	private:
		struct AnimationHeader
		{
			uint32_t perFrameTransformCount = 0;
			uint32_t frameCount = 0;
			uint32_t framesPerSecond = 0;
			float duration = 0.f;
		};
	};
}