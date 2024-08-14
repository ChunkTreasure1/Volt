#pragma once

#include "AssetSystem/AssetType.h"
#include "AssetSystem/AssetHandle.h"

#include <CoreUtilities/Assert.h>

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

	//VT_DECLARE_ASSET_TYPE(Mesh, "{5BBC2893-9D1E-47DE-AEAE-2D3D5ACB81F4}"_guid);

	//enum class AssetType : uint32_t
	//{
	//	None = 0,
	//	Mesh,
	//	MeshSource,
	//	Animation,
	//	Skeleton,
	//	Texture,
	//	Material,
	//	ShaderSource,
	//	Scene,
	//	AnimatedCharacter,
	//	ParticlePreset,
	//	Prefab,
	//	Font,
	//	PhysicsMaterial,
	//	BlendSpace,
	//	NavMesh,
	//	MonoScript,
	//	BehaviorGraph,
	//	Timeline,
	//	NetContract,
	//	PostProcessingMaterial,
	//	PostProcessingStack,
	//	MotionWeave,
	//	TextureSource,
	//	ShaderDefinition,

	//	// Add new assets above this line
	//	Count
	//};

	//inline AssetType operator|(AssetType aLhs, AssetType aRhs)
	//{
	//	return (AssetType)((std::underlying_type<AssetType>::type)aLhs | (std::underlying_type<AssetType>::type)aRhs);
	//}

	//inline AssetType operator&(AssetType aLhs, AssetType aRhs)
	//{
	//	return (AssetType)((std::underlying_type<AssetType>::type)aLhs & (std::underlying_type<AssetType>::type)aRhs);
	//}

	//inline AssetType operator~(AssetType aLhs)
	//{
	//	return (AssetType)(~(std::underlying_type<AssetType>::type)aLhs);
	//}

	//inline static std::unordered_map<std::string, AssetType> s_assetExtensionsMap =
	//{
	//	{ ".fbx", AssetType::MeshSource },
	//	{ ".gltf", AssetType::MeshSource },
	//	{ ".glb", AssetType::MeshSource },
	//	{ ".vtmesh", AssetType::Mesh },
	//	{ ".vtnavmesh", AssetType::NavMesh },

	//	{ ".vtsk", AssetType::Skeleton },
	//	{ ".vtanim", AssetType::Animation },
	//	{ ".vtchr", AssetType::AnimatedCharacter },

	//	{ ".png", AssetType::TextureSource },
	//	{ ".jpg", AssetType::TextureSource },
	//	{ ".jpeg", AssetType::TextureSource },
	//	{ ".tga", AssetType::TextureSource },
	//	{ ".ktx", AssetType::TextureSource },
	//	{ ".dds", AssetType::TextureSource },
	//	{ ".hdr", AssetType::TextureSource },

	//	{ ".vtsdef", AssetType::ShaderDefinition },
	//	{ ".hlsl", AssetType::ShaderSource },
	//	{ ".hlslh", AssetType::ShaderSource },
	//	{ ".hlsli", AssetType::ShaderSource },

	//	{ ".vtmat", AssetType::Material },
	//	{ ".vtpostmat", AssetType::PostProcessingMaterial },
	//	{ ".vtpoststack", AssetType::PostProcessingStack },
	//	{ ".vtphysmat", AssetType::PhysicsMaterial },

	//	{ ".vtscene", AssetType::Scene },
	//	{ ".vtprefab", AssetType::Prefab },
	//	{ ".vtpp", AssetType::ParticlePreset },
	//	{ ".ttf", AssetType::Font },

	//	{ ".cs", AssetType::MonoScript },
	//	{ ".vtbt", AssetType::BehaviorGraph},
	//	{ ".vtblend", AssetType::BlendSpace },

	//	{ ".vtncon", AssetType::NetContract },
	//};

	//static std::unordered_map<std::string, AssetType> s_assetNamesMap =
	//{
	//	{ "Mesh Source", AssetType::MeshSource },
	//	{ "Mesh", AssetType::Mesh },
	//	{ "NavMesh", AssetType::NavMesh },

	//	{ "Skeleton", AssetType::Skeleton },
	//	{ "Animation", AssetType::Animation },
	//	{ "Animated Character", AssetType::AnimatedCharacter },

	//	{ "TextureSource", AssetType::TextureSource },
	//	{ "Texture", AssetType::Texture },

	//	{ "Shader", AssetType::ShaderDefinition },
	//	{ "Shader Source", AssetType::ShaderSource },

	//	{ "Material", AssetType::Material },
	//	{ "Post Processing Material", AssetType::PostProcessingMaterial },
	//	{ "Post Processing Stack", AssetType::PostProcessingStack },
	//	{ "Physics Material", AssetType::PhysicsMaterial },

	//	{ "Scene", AssetType::Scene },
	//	{ "Prefab", AssetType::Prefab },
	//	{ "Particle Preset", AssetType::ParticlePreset },
	//	{ "Font", AssetType::Font },

	//	{ "Mono Script", AssetType::MonoScript },
	//	{ "Behavior Graph", AssetType::BehaviorGraph},
	//	{ "Blend Space", AssetType::BlendSpace },

	//	{ "Net Contract", AssetType::NetContract },

	//	{ "Motion Weave Database", AssetType::MotionWeave },
	//};

	//inline static const std::unordered_map<std::string, AssetType>& GetAssetNames()
	//{
	//	return s_assetNamesMap;
	//}

	//inline static std::string GetAssetTypeName(AssetType aType)
	//{
	//	for (auto& [name, type] : s_assetNamesMap)
	//	{
	//		if (type == aType)
	//			return name;
	//	}

	//	return "Unknown";
	//}

	//inline static std::string GetAssetTypeExtension(AssetType type)
	//{
	//	for (const auto& [ext, assetType] : s_assetExtensionsMap)
	//	{
	//		if (assetType == type)
	//		{
	//			return ext;
	//		}
	//	}

	//	return "Unknown";
	//}

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
