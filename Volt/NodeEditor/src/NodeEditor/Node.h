#pragma once

#include <imgui_node_editor.h>

namespace NE
{
	namespace ed = ax::NodeEditor;

	enum class PinType
	{
		Flow,
		Bool,
		Int,
		Float,
		String,
		Object,
		Function,
		Delegate,

		// Render Graph
		Framebuffer,
		ColorAttachment,
		DepthAttachment
	};

	enum class PinMode
	{
		Output,
		Input
	};

	class Node;

	struct Pin
	{
		Pin(int32_t aId, const std::string& aName, PinType aType, PinMode aMode, const std::function<void(const Pin&, bool)>& aDrawCallback = nullptr, void* aUserData = nullptr)
			: id(aId), name(aName), type(aType), mode(aMode), node(nullptr), drawCallback(aDrawCallback), userData(aUserData)
		{}

		inline static std::shared_ptr<Pin> Create(int32_t aId, const std::string& aName, PinType aType, PinMode aMode, const std::function<void(const Pin&, bool)>& aDrawCallback = nullptr, void* aUserData = nullptr)
		{
			return std::make_shared<Pin>(aId, aName, aType, aMode, aDrawCallback, aUserData);
		}

		ed::PinId id;
		Node* node;
		
		std::string name;

		PinType type;
		PinMode mode;

		void* userData = nullptr;
		std::function<void(const Pin&, bool)> drawCallback;
	};

	class Node
	{
	public:
		Node(const std::string& aName, ImColor aColor = ImColor(1.f, 1.f, 1.f));

		void Draw();

		virtual void DrawContent() {}
		virtual void OnCreate() {}
		virtual void OnLinked(const Pin* pin, const void* userData) {}
		virtual void OnUnlinked(const Pin* pin) {}

		inline const std::vector<std::shared_ptr<Pin>> GetInputPins() const { return myInputs; }
		inline const std::vector<std::shared_ptr<Pin>> GetOutputPins() const { return myOutputs; }

		inline const ed::NodeId GetId() const { return myId; }

	protected:
		void AddInput(const std::string& aName, PinType aType, void* aUserData = nullptr, const std::function<void(const Pin&, bool)>& aDrawCallback = nullptr);
		void AddOutput(const std::string& aName, PinType aType, void* aUserData = nullptr, const std::function<void(const Pin&, bool)>& aDrawCallback = nullptr);

		friend class Graph;

		std::string myName;
		ed::NodeId myId = 0;

		std::vector<std::shared_ptr<Pin>> myInputs;
		std::vector<std::shared_ptr<Pin>> myOutputs;

		ImColor myColor;
		ImVec2 mySize;

		Graph* myGraph = nullptr;
	};

	struct Link
	{
		Link(ed::LinkId aId, ed::PinId aStartId, ed::PinId aEndId)
			: id(aId), startPin(aStartId), endPin(aEndId)
		{}

		ed::LinkId id;

		ed::PinId startPin;
		ed::PinId endPin;
		
		ImColor color;
	};
}