#pragma once

#include "AssetSystem/AssetType.h"
#include "AssetSystem/AssetHandle.h"

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

	VT_SETUP_ENUM_CLASS_OPERATORS(AssetFlag);

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

		VT_NODISCARD VT_INLINE bool IsValid() const { return ((assetFlags & AssetFlag::Missing) | (assetFlags & AssetFlag::Invalid) | (assetFlags & AssetFlag::Queued)) == AssetFlag::None; }

		inline virtual bool operator==(const Asset& other)
		{
			return handle = other.handle;
		}

		inline virtual bool operator!=(const Asset& other)
		{
			return !(*this == other);
		}

		VT_NODISCARD VT_INLINE bool IsFlagSet(AssetFlag flag) { return (assetFlags & flag) != AssetFlag::None; }
		VT_INLINE void SetFlag(AssetFlag flag, bool state)
		{
			if (state)
			{
				assetFlags |= flag;
			}
			else
			{
				assetFlags &= ~flag;
			}
		}

		VT_NODISCARD VT_INLINE static const AssetHandle Null() { return AssetHandle(0); }

		virtual AssetType GetType() { return AssetTypes::None; }
		virtual uint32_t GetVersion() const { return 1; }
		virtual void OnDependencyChanged(AssetHandle dependencyHandle, AssetChangedState state) {}

		AssetFlag assetFlags = AssetFlag::None;
		AssetHandle handle = {};
		std::string assetName;
	};
}
