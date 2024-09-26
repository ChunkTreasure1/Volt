#pragma once

#include "Volt/Asset/AssetTypes.h"

#include <AssetSystem/Asset.h>

namespace Volt
{
	class PhysicsMaterial : public Asset
	{
	public:
		float staticFriction = 0.1f;
		float dynamicFriction = 0.1f;
		float bounciness = 1.f;

		static AssetType GetStaticType() { return AssetTypes::PhysicsMaterial; }
		AssetType GetType() override { return GetStaticType(); };
		uint32_t GetVersion() const override { return 1; }
	};
}
