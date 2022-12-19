#pragma once

#include <Volt/Core/Base.h>

#include <unordered_map>
#include <functional>
#include <memory>

#define GK_REGISTER_NODE_SPECIALIZED(name, node) \
	inline static bool name ## _entry = GraphKey::Registry::Register(#name, [](){ return std::make_shared<node>(); });

#define GK_REGISTER_NODE(node) \
	inline static bool node ## _entry = GraphKey::Registry::Register(#node, [](){ return std::make_shared<node>(); });

namespace GraphKey
{
	struct Node;
	class Registry
	{
	public:
		static bool Register(const std::string& name, std::function<Ref<Node>()>&& createFunction);
		static Ref<Node> Create(const std::string& name);
		
		inline static const std::unordered_map<std::string, std::function<Ref<Node>()>>& GetRegistry() { return myRegistry; }

	private:
		Registry() = delete;

		inline static std::unordered_map<std::string, std::function<Ref<Node>()>> myRegistry;
	};
}