#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class AssetPacker
	{
	public:
		struct AssetPackHeader
		{
			size_t assetCount = 0;
			size_t version = 0;
		};

		struct AssetHeader
		{
			AssetHandle handle = Asset::Null();
			AssetType type = AssetType::None;
			size_t size = 0;
		};

	private:

		AssetPacker() = delete;
	};
}