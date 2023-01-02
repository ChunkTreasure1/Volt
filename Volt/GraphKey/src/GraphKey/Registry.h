#pragma once

#include <Volt/Core/Base.h>

#include <unordered_map>
#include <functional>
#include <memory>

#define UNPACK(...) __VA_ARGS__
#define GK_REGISTER_NODE_SPECIALIZED(name, category, node) \
	inline static bool name ## _entry = GraphKey::Registry::Register(#name, category, [](){ return std::make_shared<UNPACK node>(); });

#define GK_REGISTER_NODE(node, category) \
	inline static bool node ## _entry = GraphKey::Registry::Register(#node, category, [](){ return std::make_shared<node>(); });

namespace GraphKey
{
	struct Node;
	class Registry
	{
	public:
		static bool Register(const std::string& name, const std::string& category, std::function<Ref<Node>()>&& createFunction);
		static Ref<Node> Create(const std::string& name);
		static const std::string& GetCategory(const std::string& name);

		inline static const std::unordered_map<std::string, std::function<Ref<Node>()>>& GetRegistry() { return myRegistry; }

	private:
		Registry() = delete;

		inline static std::unordered_map<std::string, std::function<Ref<Node>()>> myRegistry;
		inline static std::unordered_map<std::string, std::string> myCategories;
	};
}