#pragma once

#include "GraphKey/TypeTraits.h"

#include <Volt/Core/Base.h>
#include <CoreUtilities/Containers/Vector.h>

#include <unordered_map>
#include <functional>
#include <memory>

#define UNPACK(...) __VA_ARGS__
#define GK_REGISTER_NODE_SPECIALIZED(name, category, editorType, node) \
	static bool name ## _entry = GraphKey::Registry::Register(#name, category, editorType, true, [](){ return std::make_shared<UNPACK node>(); });

#define GK_REGISTER_NODE_SPECIALIZED_ANON(name, category, editorType, node) \
	static bool name ## _entry = GraphKey::Registry::Register(#name, category, editorType, false, [](){ return std::make_shared<UNPACK node>(); });

#define GK_REGISTER_NODE(node, category, editorType) \
	static bool node ## _entry = GraphKey::Registry::Register(#node, category, editorType, true, [](){ return std::make_shared<node>(); });

#define GK_REGISTER_NODE_ANON(node, category, editorType) \
	static bool node ## _entry = GraphKey::Registry::Register(#node, category, editorType, false, [](){ return std::make_shared<node>(); });

#define GK_REGISTER_PARAMETER_TYPE(type, typeName, uiFunction) \
	GK_REGISTER_TYPE(type, typeName, uiFunction) \
	GK_REGISTER_NODE_SPECIALIZED_ANON(SetParameter ## typeName, "Parameters", GraphType::All, (SetParameterNode<type>)); \
	GK_REGISTER_NODE_SPECIALIZED_ANON(GetParameter ## typeName, "Parameters", GraphType::All, (GetParameterNode<type>));

#define GK_CREATE_GET_PARAMETER_NODE(typeIndex, paramId, outNode) \
	{												  \
		outNode = GraphKey::Registry::Create("GetParameter" + GraphKey::TypeRegistry::GetNameFromTypeIndex(typeIndex)); \
		auto paramNode = std::reinterpret_pointer_cast<GraphKey::ParameterNode>(node);						\
		paramNode->parameterId = paramId; \
	}

#define GK_CREATE_SET_PARAMETER_NODE(typeIndex, paramId, outNode) \
	{												  \
		outNode = GraphKey::Registry::Create("SetParameter" + GraphKey::TypeRegistry::GetNameFromTypeIndex(typeIndex)); \
		auto paramNode = std::reinterpret_pointer_cast<GraphKey::ParameterNode>(node);						\
		paramNode->parameterId = paramId; \
	}\

namespace GraphKey
{
	enum class GraphType
	{
		Animation,
		Scripting,
		Material,
		All
	};

	struct Node;
	struct RegistryEntry
	{
		std::string name;
		std::string category;
		std::function<Ref<Node>()> createFunc;
		GraphType graphType;
		bool visible = true;
	};

	class Registry
	{
	public:
		static bool Register(const std::string& name, const std::string& category, GraphType editorType, bool visible, std::function<Ref<Node>()>&& createFunction);
		static Ref<Node> Create(const std::string& name);
		static const std::string& GetCategory(const std::string& name);

		inline static const std::unordered_map<std::string, RegistryEntry>& GetRegistry() { return myRegistry; }
		static const Vector<RegistryEntry> GetNodesOfGraphType(GraphType type);

	private:
		Registry() = delete;

		inline static std::unordered_map<std::string, RegistryEntry> myRegistry;
		inline static std::unordered_map<std::string, std::string> myCategories;
	};
}
