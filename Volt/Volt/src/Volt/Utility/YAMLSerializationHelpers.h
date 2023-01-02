#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Rendering/Buffer/BufferLayout.h"

#include <gem/gem.h>
#include <Wire/WireGUID.h>
#include <Wire/Serialization.h>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	template<>
	struct convert<gem::vec2>
	{
		static Node encode(const gem::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, gem::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<gem::vec3>
	{
		static Node encode(const gem::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, gem::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<gem::vec4>
	{
		static Node encode(const gem::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, gem::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<gem::vec2ui>
	{
		static Node encode(const gem::vec2ui& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, gem::vec2ui& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<uint32_t>();
			rhs.y = node[1].as<uint32_t>();
			return true;
		}
	};

	template<>
	struct convert<gem::vec3ui>
	{
		static Node encode(const gem::vec3ui& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, gem::vec3ui& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<uint32_t>();
			rhs.y = node[1].as<uint32_t>();
			rhs.z = node[2].as<uint32_t>();
			return true;
		}
	};

	template<>
	struct convert<gem::vec4ui>
	{
		static Node encode(const gem::vec4ui& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, gem::vec4ui& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<uint32_t>();
			rhs.y = node[1].as<uint32_t>();
			rhs.z = node[2].as<uint32_t>();
			rhs.w = node[3].as<uint32_t>();
			return true;
		}
	};

	template<>
	struct convert<gem::quat>
	{
		static Node encode(const gem::quat& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, gem::quat& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<gem::mat4>
	{
		static Node encode(const gem::mat4& rhs)
		{
			Node node;
			node.push_back(rhs[0][0]);
			node.push_back(rhs[0][1]);
			node.push_back(rhs[0][2]);
			node.push_back(rhs[0][3]);

			node.push_back(rhs[1][0]);
			node.push_back(rhs[1][1]);
			node.push_back(rhs[1][2]);
			node.push_back(rhs[1][3]);

			node.push_back(rhs[2][0]);
			node.push_back(rhs[2][1]);
			node.push_back(rhs[2][2]);
			node.push_back(rhs[2][3]);

			node.push_back(rhs[3][0]);
			node.push_back(rhs[3][1]);
			node.push_back(rhs[3][2]);
			node.push_back(rhs[3][3]);
			return node;
		}

		static bool decode(const Node& node, gem::mat4& rhs)
		{
			if (!node.IsSequence() || node.size() != 16)
			{
				return false;
			}

			rhs[0][0] = node[0].as<float>();
			rhs[0][1] = node[1].as<float>();
			rhs[0][2] = node[2].as<float>();
			rhs[0][3] = node[3].as<float>();

			rhs[1][0] = node[4].as<float>();
			rhs[1][1] = node[5].as<float>();
			rhs[1][2] = node[6].as<float>();
			rhs[1][3] = node[7].as<float>();

			rhs[2][0] = node[8].as<float>();
			rhs[2][1] = node[9].as<float>();
			rhs[2][2] = node[10].as<float>();
			rhs[2][3] = node[11].as<float>();

			rhs[3][0] = node[12].as<float>();
			rhs[3][1] = node[13].as<float>();
			rhs[3][2] = node[14].as<float>();
			rhs[3][3] = node[15].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Volt::AssetHandle>
	{
		static Node encode(const Volt::AssetHandle& rhs)
		{
			Node node;
			node.push_back((uint64_t)rhs);
			return node;
		};

		static bool decode(const Node& node, Volt::AssetHandle& v)
		{
			v = node.as<uint64_t>();
			return true;
		};
	};

	template<>
	struct convert<WireGUID>
	{
		static Node encode(const WireGUID& rhs)
		{
			Node node;
			node.push_back(rhs.hiPart);
			node.push_back(rhs.loPart);
			return node;
		};

		static bool decode(const Node& node, WireGUID& v)
		{
			WireGUID guid{ node[0].as<uint64_t>(), node[1].as<uint64_t>() };
			v = guid;
			return true;
		};
	};

	template<>
	struct convert<Volt::ElementType>
	{
		static Node encode(const Volt::ElementType& rhs)
		{
			Node node;
			node.push_back((uint32_t)rhs);
			return node;
		};

		static bool decode(const Node& node, Volt::ElementType& v)
		{
			v = (Volt::ElementType)node.as<uint32_t>();
			return true;
		};
	};

	template<>
	struct convert<std::filesystem::path>
	{
		static Node encode(const std::filesystem::path& rhs)
		{
			Node node;
			node.push_back(rhs.string());
			return node;
		};

		static bool decode(const Node& node, std::filesystem::path& v)
		{
			v = node.as<std::string>();
			return true;
		};
	};

	template<>
	struct convert<Wire::ComponentRegistry::PropertyType>
	{
		static Node encode(const Wire::ComponentRegistry::PropertyType& rhs)
		{
			Node node;
			node.push_back((uint32_t)rhs);
			return node;
		};

		static bool decode(const Node& node, Wire::ComponentRegistry::PropertyType& v)
		{
			v = (Wire::ComponentRegistry::PropertyType)node.as<uint32_t>();
			return true;
		};
	};
}

namespace Volt
{
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const gem::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const gem::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const gem::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const gem::vec2ui& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const gem::vec3ui& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const gem::vec4ui& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const gem::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const gem::mat4& mat)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq
			<< mat[0][0] << mat[0][1] << mat[0][2] << mat[0][3]
			<< mat[1][0] << mat[1][1] << mat[1][2] << mat[1][3]
			<< mat[2][0] << mat[2][1] << mat[2][2] << mat[2][3]
			<< mat[3][0] << mat[3][1] << mat[3][2] << mat[3][3]
			<< YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Volt::AssetHandle& handle)
	{
		out << static_cast<uint64_t>(handle);
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Volt::ElementType& handle)
	{
		out << static_cast<uint32_t>(handle);
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Wire::ComponentRegistry::PropertyType& handle)
	{
		out << static_cast<uint32_t>(handle);
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const WireGUID& handle)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << handle.hiPart << handle.loPart << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const std::filesystem::path& path)
	{
		out << path.string();
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<std::filesystem::path>& paths)
	{
		out << YAML::BeginSeq;
		for (const auto& p : paths)
		{
			out << p;
		}
		out << YAML::EndSeq;
		return out;
	}
}