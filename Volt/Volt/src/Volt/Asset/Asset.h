#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/UUID.h"

#include <unordered_map>
#include <filesystem>

#include <cassert>
#include <any>


namespace Volt
{
	using AssetHandle = UUID64;

	enum class AssetFlag : uint16_t
	{
		None = 0,
		Missing = BIT(0),
		Invalid = BIT(1),
		Queued = BIT(2)
	};

	enum class AssetType : uint32_t
	{
		None = 0,
		Mesh = BIT(0),
		MeshSource = BIT(1),
		Animation = BIT(2),
		Skeleton = BIT(3),
		Texture = BIT(4),
		Material = BIT(5),
		ShaderDefinition = BIT(6),
		ShaderSource = BIT(7),
		Scene = BIT(8),
		AnimatedCharacter = BIT(9),
		ParticlePreset = BIT(10),
		Prefab = BIT(11),
		Font = BIT(12),
		PhysicsMaterial = BIT(13),
		Video = BIT(14),
		BlendSpace = BIT(15),
		NavMesh = BIT(16),
		AnimationGraph = BIT(17),
		GraphKey = BIT(18),
		MonoScript = BIT(19),
		BehaviorGraph = BIT(20),
		MaterialGraph = BIT(21),
		RenderPipeline = BIT(22),
		Timeline = BIT(23),
		NetContract = BIT(24),
		PostProcessingMaterial = BIT(25),
		PostProcessingStack = BIT(26),
		MotionWeave = BIT(27),
	};

	inline AssetType operator|(AssetType aLhs, AssetType aRhs)
	{
		return (AssetType)((std::underlying_type<AssetType>::type)aLhs | (std::underlying_type<AssetType>::type)aRhs);
	}

	inline AssetType operator&(AssetType aLhs, AssetType aRhs)
	{
		return (AssetType)((std::underlying_type<AssetType>::type)aLhs & (std::underlying_type<AssetType>::type)aRhs);
	}

	inline AssetType operator~(AssetType aLhs)
	{
		return (AssetType)(~(std::underlying_type<AssetType>::type)aLhs);
	}

	inline static std::unordered_map<std::string, AssetType> s_assetExtensionsMap =
	{
		{ ".fbx", AssetType::MeshSource },
		{ ".gltf", AssetType::MeshSource },
		{ ".glb", AssetType::MeshSource },
		{ ".vtmesh", AssetType::Mesh },
		{ ".vtnavmesh", AssetType::NavMesh },

		{ ".vtsk", AssetType::Skeleton },
		{ ".vtanim", AssetType::Animation },
		{ ".vtchr", AssetType::AnimatedCharacter },
		{ ".vtanimgraph", AssetType::AnimationGraph },

		{ ".png", AssetType::Texture },
		{ ".jpg", AssetType::Texture },
		{ ".jpeg", AssetType::Texture },
		{ ".tga", AssetType::Texture },
		{ ".ktx", AssetType::Texture },
		{ ".dds", AssetType::Texture },
		{ ".hdr", AssetType::Texture },

		{ ".vtsdef", AssetType::ShaderDefinition },
		{ ".hlsl", AssetType::ShaderSource },
		{ ".hlslh", AssetType::ShaderSource },
		{ ".hlsli", AssetType::ShaderSource },

		{ ".vtmgraph", AssetType::MaterialGraph },
		{ ".vtmat", AssetType::Material },
		{ ".vtpostmat", AssetType::PostProcessingMaterial },
		{ ".vtpoststack", AssetType::PostProcessingStack },
		{ ".vtphysmat", AssetType::PhysicsMaterial },

		{ ".vtscene", AssetType::Scene },
		{ ".vtprefab", AssetType::Prefab },
		{ ".vtpp", AssetType::ParticlePreset },
		{ ".ttf", AssetType::Font },

		{ ".mp4", AssetType::Video },
		{ ".vtrp", AssetType::RenderPipeline },
		{ ".vtgk", AssetType::GraphKey },

		{ ".cs", AssetType::MonoScript },
		{ ".vtbt", AssetType::BehaviorGraph},
		{ ".vtblend", AssetType::BlendSpace },

		{ ".vtncon", AssetType::NetContract },
	};

	static std::unordered_map<std::string, AssetType> s_assetNamesMap =
	{
		{ "Mesh Source", AssetType::MeshSource },
		{ "Mesh", AssetType::Mesh },
		{ "NavMesh", AssetType::NavMesh },

		{ "Skeleton", AssetType::Skeleton },
		{ "Animation", AssetType::Animation },
		{ "Animated Character", AssetType::AnimatedCharacter },
		{ "Animation Graph", AssetType::AnimationGraph },

		{ "Texture", AssetType::Texture },

		{ "Shader", AssetType::ShaderDefinition },
		{ "Shader Source", AssetType::ShaderSource },

		{ "Material Graph", AssetType::MaterialGraph },
		{ "Material", AssetType::Material },
		{ "Post Processing Material", AssetType::PostProcessingMaterial },
		{ "Post Processing Stack", AssetType::PostProcessingStack },
		{ "Physics Material", AssetType::PhysicsMaterial },

		{ "Scene", AssetType::Scene },
		{ "Prefab", AssetType::Prefab },
		{ "Particle Preset", AssetType::ParticlePreset },
		{ "Font", AssetType::Font },
		{ "Video", AssetType::Video },

		{ "Mono Script", AssetType::MonoScript },
		{ "Behavior Graph", AssetType::BehaviorGraph},
		{ "Blend Space", AssetType::BlendSpace },

		{ "Net Contract", AssetType::NetContract }
	};

	inline static const std::unordered_map<std::string, AssetType>& GetAssetNames()
	{
		return s_assetNamesMap;
	}

	inline static std::string GetAssetTypeName(AssetType aType)
	{
		for (auto& [name, type] : s_assetNamesMap)
		{
			if (type == aType)
				return name;
		}

		return "Unknown";
	}

	struct AssetMetadata
	{
		inline void SetValue(const std::string& key, const std::string& data)
		{
			properties[key] = data;
		}

		inline const std::string& GetValue(const std::string& key) const
		{
			if (!properties.contains(key))
			{
				return {};
			}

			return properties.at(key);
		}

		inline const bool IsValid() const { return handle != 0; }

		AssetHandle handle = 0;
		AssetType type = AssetType::None;

		bool isLoaded = false;
		bool isQueued = false;
		bool isMemoryAsset = false;

		std::filesystem::path filePath;
		std::vector<AssetHandle> dependencies;
		std::unordered_map<std::string, std::string> properties;
	};

	class Asset
	{
	public:
		virtual ~Asset() = default;

		inline bool IsValid() const { return ((assetFlags & (uint16_t)AssetFlag::Missing) | (assetFlags & (uint16_t)AssetFlag::Invalid) | (assetFlags & (uint16_t)AssetFlag::Queued)) == 0; }

		inline virtual bool operator==(const Asset& other)
		{
			return handle = other.handle;
		}

		inline virtual bool operator!=(const Asset& other)
		{
			return !(*this == other);
		}

		inline bool IsFlagSet(AssetFlag flag) { return (assetFlags & (uint16_t)flag) != 0; }
		inline void SetFlag(AssetFlag flag, bool state)
		{
			if (state)
			{
				assetFlags |= (uint16_t)flag;
			}
			else
			{
				assetFlags &= ~(uint16_t)flag;
			}
		}

		inline static const AssetHandle Null() { return AssetHandle(0); }

		static AssetType GetStaticType() { return AssetType::None; }
		virtual AssetType GetType() { assert(false); return AssetType::None; }

		uint16_t assetFlags = (uint16_t)AssetFlag::None;
		AssetHandle handle = {};
		std::string assetName;
	};
}
