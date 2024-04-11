#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/TimelinePreset.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"

#include <CoreUtilities/VoltGUID.h>
#include <CoreUtilities/FileIO/SerializationHelpers.h>

#include <entt.hpp>

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	template<>
	struct convert<Volt::EntityID>
	{
		static Node encode(const Volt::EntityID& rhs)
		{
			Node node;
			node.push_back((uint32_t)rhs);
			return node;
		};

		static bool decode(const Node& node, Volt::EntityID& v)
		{
			v = node.as<uint32_t>();
			return true;
		};
	};

	template<>
	struct convert<entt::entity>
	{
		static Node encode(const entt::entity& rhs)
		{
			Node node;
			node.push_back((uint32_t)rhs);
			return node;
		};

		static bool decode(const Node& node, entt::entity& v)
		{
			v = static_cast<entt::entity>(node.as<uint32_t>());
			return true;
		};
	};

	template<>
	struct convert<Volt::TrackType>
	{
		static Node encode(const Volt::TrackType& rhs)
		{
			Node node;
			node.push_back((uint32_t)rhs);
			return node;
		};

		static bool decode(const Node& node, Volt::TrackType& v)
		{
			v = (Volt::TrackType)node.as<uint32_t>();
			return true;
		};
	};

	template<>
	struct convert<Volt::KeyframeType>
	{
		static Node encode(const Volt::KeyframeType& rhs)
		{
			Node node;
			node.push_back((uint32_t)rhs);
			return node;
		};

		static bool decode(const Node& node, Volt::KeyframeType& v)
		{
			v = (Volt::KeyframeType)node.as<uint32_t>();
			return true;
		};
	};
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const Volt::AssetHandle& handle)
{
	out << static_cast<uint64_t>(handle);
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const entt::entity& handle)
{
	out << static_cast<uint32_t>(handle);
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const Volt::TrackType& handle)
{
	out << static_cast<uint32_t>(handle);
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const Volt::KeyframeType& handle)
{
	out << static_cast<uint32_t>(handle);
	return out;
}
