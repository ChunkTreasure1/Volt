#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class Texture2D;
}

class AssetIconLibrary
{
public:
	static void Initialize();
	static void Shutdowm();

	static Ref<Volt::Texture2D> Get(Volt::AssetType type);

private:
	inline static std::unordered_map<Volt::AssetType, Ref<Volt::Texture2D>> myAssetIcons;

	AssetIconLibrary() = delete;
};