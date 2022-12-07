#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/UUID.h"

#include <unordered_map>
#include <filesystem>

namespace Volt
{
	typedef UUID AssetHandle;

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
		Shader = BIT(6),
		ShaderSource = BIT(7),
		Scene = BIT(8),
		AnimatedCharacter = BIT(9),
		ParticlePreset = BIT(10),
		Prefab = BIT(11),
		Font = BIT(12),
		PhysicsMaterial = BIT(13),
		Video = BIT(14),
		RenderPipeline = BIT(15)
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

		{ ".vtsk", AssetType::Skeleton },
		{ ".vtanim", AssetType::Animation },
		{ ".vtchr", AssetType::AnimatedCharacter },

		{ ".png", AssetType::Texture },
		{ ".jpg", AssetType::Texture },
		{ ".jpeg", AssetType::Texture },
		{ ".tga", AssetType::Texture },
		{ ".ktx", AssetType::Texture },
		{ ".dds", AssetType::Texture },
		{ ".hdr", AssetType::Texture },

		{ ".vtsdef", AssetType::Shader },
		{ ".hlsl", AssetType::ShaderSource },
		{ ".hlslh", AssetType::ShaderSource },

		{ ".vtmat", AssetType::Material },
		{ ".vtphysmat", AssetType::PhysicsMaterial },

		{ ".vtscene", AssetType::Scene },
		{ ".vtprefab", AssetType::Prefab },
		{ ".vtpp", AssetType::ParticlePreset },
		{ ".ttf", AssetType::Font },

		{ ".mp4", AssetType::Video },
		{ ".vtpipeline", AssetType::RenderPipeline }
	};

	inline static std::unordered_map<std::string, AssetType> s_assetNamesMap =
	{
		{ "Mesh Source", AssetType::MeshSource },
		{ "Mesh", AssetType::Mesh },

		{ "Skeleton", AssetType::Skeleton },
		{ "Animation", AssetType::Animation },
		{ "Animated Character", AssetType::AnimatedCharacter },

		{ "Texture", AssetType::Texture },

		{ "Shader", AssetType::Shader },
		{ "Shader Source", AssetType::ShaderSource },

		{ "Material", AssetType::Material },
		{ "Physics Material", AssetType::PhysicsMaterial },

		{ "Scene", AssetType::Scene },
		{ "Prefab", AssetType::Prefab },
		{ "Particle Preset", AssetType::ParticlePreset },
		{ "Font", AssetType::Font },
		{ "Video", AssetType::Video },
		{ "Render Pipeline", AssetType::RenderPipeline }
	};

	class Asset
	{
	public:
		virtual ~Asset() = default;

		inline bool IsValid() const { return ((flags & (uint16_t)AssetFlag::Missing) | (flags & (uint16_t)AssetFlag::Invalid) | (flags & (uint16_t)AssetFlag::Queued)) == 0; }

		inline virtual bool operator==(const Asset& other)
		{
			return handle = other.handle;
		}

		inline virtual bool operator!=(const Asset& other)
		{
			return !(*this == other);
		}

		inline bool IsFlagSet(AssetFlag flag) { return (flags & (uint16_t)flag) != 0; }
		inline void SetFlag(AssetFlag flag, bool state)
		{
			if (state)
			{
				flags |= (uint16_t)flag;
			}
			else
			{
				flags &= ~(uint16_t)flag;
			}
		}

		inline static const AssetHandle Null() { return AssetHandle(0); }

		static AssetType GetStaticType() { return AssetType::None; }
		virtual AssetType GetType() { return AssetType::None; }

		uint16_t flags = (uint16_t)AssetFlag::None;
		AssetHandle handle = {};
		std::filesystem::path path;
	};
}