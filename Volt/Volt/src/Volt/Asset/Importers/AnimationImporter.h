#pragma once

#include "Volt/Asset/Importers/AssetImporter.h"

namespace Volt
{
	class AnimationImporter : public AssetImporter
	{
	public:
		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;

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
