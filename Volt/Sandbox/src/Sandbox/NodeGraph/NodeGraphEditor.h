#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Core/Base.h>
#include <Volt/Core/UUID.h>

#include <imgui_node_editor.h>
#include <functional>
#include <cstdint>

namespace NodeGraph
{
	struct EditorBackend;

	struct EditorContext
	{
		ax::NodeEditor::EditorContext* editorContext = nullptr;
		ax::NodeEditor::NodeId contextNodeId;
		ax::NodeEditor::PinId contextPinId;
		ax::NodeEditor::LinkId contextLinkId;

		std::function<void()> onBeginCreate;

		std::function<void(const UUID64)> onDeleteLink;
		std::function<void(const UUID64)> onDeleteNode;

		std::function<void()> onShowNodeContextMenu;
		std::function<void()> onShowPinContextMenu;
		std::function<void()> onShowLinkContextMenu;
		std::function<void()> onShowBackgroundContextMenu;

		std::function<void()> onAcceptCopy;
		std::function<void()> onAcceptCut;
		std::function<void()> onAcceptPaste;
		std::function<void()> onAcceptDuplicate;
	};

	class Editor : public EditorWindow
	{
	public:
		Editor(const std::string& title, const std::string& context, bool dockSpace, Ref<EditorBackend> backend);
		~Editor() override;

		void Update();

		void UpdateMainContent() override;
		void UpdateContent() override;

		void CreateNode(const UUID64 id, const std::vector<UUID64> pins);
		void CreateLink(const UUID64 id, const UUID64 input, const UUID64 output);

		void RemoveLink(const UUID64 linkId);
		void RemoveNode(const UUID64 nodeId);

		const std::vector<UUID64> GetSelectedNodes();
		const std::vector<UUID64> GetSelectedLinks();

		virtual bool SaveSettings(const std::string& data) = 0;
		virtual size_t LoadSettings(std::string& data)  = 0;

		virtual bool SaveNodeSettings(const UUID64 nodeId, const std::string& data) = 0;
		virtual size_t LoadNodeSettings(const UUID64 nodeId, std::string& data) = 0;

		inline const EditorContext& GetContext() const { return *myEditorContext; }
		inline EditorBackend& GetBackend() const { return *myBackend; }

		inline void SetGraphTypeText(const std::string& text) { myGraphTypeText = text; }

	protected:
		virtual void InitializeStyle(ax::NodeEditor::Style& editorStyle) {}
		void InitializeEditor(Ref<EditorContext> context);
		void DrawEditor();

		virtual void DrawPanels() {};
		virtual void DrawMenuBar() {}
		virtual void DrawContextPopups() {}
		virtual void DrawNodes() = 0;
		virtual void DrawLinks();

		bool myCreateNewNode = false;
		UUID64 myNewLinkPinId = 0;
		std::string myContext;

	private:
		std::string myGraphTypeText;

		Ref<EditorContext> myEditorContext;
		Ref<EditorBackend> myBackend;
	};
}
