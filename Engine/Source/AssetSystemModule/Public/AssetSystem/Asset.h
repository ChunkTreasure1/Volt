#pragma once

#include "AssetSystem/AssetType.h"
#include "AssetSystem/AssetHandle.h"

#include <CoreUtilities/VoltAssert.h>

#include <unordered_map>
#include <filesystem>

namespace Volt
{
	enum class AssetChangedState : uint8_t
	{
		Removed,
		Updated
	};

	enum class AssetFlag : uint8_t
	{
		None = 0,
		Missing = BIT(0),
		Invalid = BIT(1),
		Queued = BIT(2)
	};

	struct AssetMetadata
	{
		inline const bool IsValid() const { return handle != 0; }

		AssetHandle handle = 0;
		AssetType type;

		bool isLoaded = false;
		bool isQueued = false;
		bool isMemoryAsset = false;

		std::filesystem::path filePath;
	};

	class Asset
	{
	public:
		virtual ~Asset() = default;

		inline bool IsValid() const { return ((assetFlags & (uint8_t)AssetFlag::Missing) | (assetFlags & (uint8_t)AssetFlag::Invalid) | (assetFlags & (uint8_t)AssetFlag::Queued)) == 0; }

		inline virtual bool operator==(const Asset& other)
		{
			return handle = other.handle;
		}

		inline virtual bool operator!=(const Asset& other)
		{
			return !(*this == other);
		}

		inline bool IsFlagSet(AssetFlag flag) { return (assetFlags & (uint8_t)flag) != 0; }
		inline void SetFlag(AssetFlag flag, bool state)
		{
			if (state)
			{
				assetFlags |= (uint8_t)flag;
			}
			else
			{
				assetFlags &= ~(uint8_t)flag;
			}
		}

		inline static const AssetHandle Null() { return AssetHandle(0); }

		virtual AssetType GetType() { return AssetTypes::None; }
		virtual uint32_t GetVersion() const { return 1; }
		virtual void OnDependencyChanged(AssetHandle dependencyHandle, AssetChangedState state) {}

		uint8_t assetFlags = (uint8_t)AssetFlag::None;
		AssetHandle handle = {};
		std::string assetName;
	};
}
