#pragma once

#include "Volt/Rendering/Buffer/BufferLayout.h"
#include "Volt/Rendering/Shader/Shader.h"

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/TimelinePreset.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"

#include "Volt/Scene/Reflection/VoltGUID.h"

#include <entt.hpp>

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
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
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
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
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
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
	struct convert<glm::uvec2>
	{
		static Node encode(const glm::uvec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::uvec2& rhs)
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
	struct convert<glm::uvec3>
	{
		static Node encode(const glm::uvec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::uvec3& rhs)
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
	struct convert<glm::uvec4>
	{
		static Node encode(const glm::uvec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::uvec4& rhs)
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
	struct convert<glm::ivec2>
	{
		static Node encode(const glm::ivec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::ivec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<int32_t>();
			rhs.y = node[1].as<int32_t>();
			return true;
		}
	};

	template<>
	struct convert<glm::ivec3>
	{
		static Node encode(const glm::ivec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::ivec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<int32_t>();
			rhs.y = node[1].as<int32_t>();
			rhs.z = node[2].as<int32_t>();
			return true;
		}
	};

	template<>
	struct convert<glm::ivec4>
	{
		static Node encode(const glm::ivec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::ivec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<int32_t>();
			rhs.y = node[1].as<int32_t>();
			rhs.z = node[2].as<int32_t>();
			rhs.w = node[3].as<int32_t>();
			return true;
		}
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs)
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
	struct convert<glm::mat4>
	{
		static Node encode(const glm::mat4& rhs)
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

		static bool decode(const Node& node, glm::mat4& rhs)
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
	struct convert<VoltGUID>
	{
		static Node encode(const VoltGUID& rhs)
		{
			Node node;
			node.push_back(rhs.hiPart);
			node.push_back(rhs.loPart);
			return node;
		};

		static bool decode(const Node& node, VoltGUID& v)
		{
			VoltGUID guid{ node[0].as<uint64_t>(), node[1].as<uint64_t>() };
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
	struct convert<Volt::ShaderUniformType>
	{
		static Node encode(const Volt::ShaderUniformType& rhs)
		{
			Node node;
			node.push_back((uint32_t)rhs);
			return node;
		};

		static bool decode(const Node& node, Volt::ShaderUniformType& v)
		{
			v = (Volt::ShaderUniformType)node.as<uint32_t>();
			return true;
		};
	};

	template<>
	struct convert<Volt::ShaderStage>
	{
		static Node encode(const Volt::ShaderStage& rhs)
		{
			Node node;
			node.push_back((uint32_t)rhs);
			return node;
		};

		static bool decode(const Node& node, Volt::ShaderStage& v)
		{
			v = (Volt::ShaderStage)node.as<uint32_t>();
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
}

namespace Volt
{
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::uvec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::uvec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::uvec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::ivec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::ivec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::ivec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& mat)
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

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Volt::ElementType& handle)
	{
		out << static_cast<uint32_t>(handle);
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Volt::ShaderUniformType& type)
	{
		out << static_cast<uint32_t>(type);
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Volt::ShaderStage& type)
	{
		out << static_cast<uint32_t>(type);
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const VoltGUID& handle)
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
