#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class PhysicsMaterial : public Asset
	{
	public:
		float staticFriction = 0.1f;
		float dynamicFriction = 0.1f;
		float bounciness = 1.f;

		static AssetType GetStaticType() { return AssetType::PhysicsMaterial; }
		AssetType GetType() override { return GetStaticType(); };
	};
}