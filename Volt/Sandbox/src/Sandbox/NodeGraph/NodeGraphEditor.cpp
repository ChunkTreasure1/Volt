#include "sbpch.h"
#include "NodeGraphEditor.h"

#include "Sandbox/NodeGraph/NodeGraphEditorBackend.h"

#include <Volt/Utility/UIUtility.h>

namespace ed = ax::NodeEditor;

namespace NodeGraph
{
	Editor::Editor(const std::string& title, const std::string& context, bool dockSpace, Ref<EditorBackend> backend)
		: EditorWindow(title, dockSpace), myContext(context), myBackend(backend)
	{
		myWindowFlags = ImGuiWindowFlags_MenuBar;
	}

	Editor::~Editor()
	{
		if (myEditorContext->editorContext)
		{
			ed::DestroyEditor(myEditorContext->editorContext);
		}
	}

	void Editor::Update()
	{
	}

	void Editor::UpdateMainContent()
	{
		if (myHasDockSpace)
		{
			DrawMenuBar();
		}
		else
		{
			DrawEditor();
		}
	}

	void Editor::UpdateContent()
	{
		if (myHasDockSpace)
		{
			DrawEditor();

			ed::SetCurrentEditor(myEditorContext->editorContext);
			DrawPanels();
			ed::SetCurrentEditor(nullptr);
		}
	}

	void Editor::CreateNode(const Volt::UUID id, const std::vector<Volt::UUID> pins)
	{
		Node newNode{ id, pins };
		GetBackend().nodes.emplace_back(newNode);
	}

	void Editor::CreateLink(const Volt::UUID id, const Volt::UUID input, const Volt::UUID output)
	{
		Link newLink{};
		newLink.id = id;
		newLink.input = input;
		newLink.output = output;

		GetBackend().links.emplace_back(newLink);
	}

	void Editor::RemoveLink(const Volt::UUID linkId)
	{
		auto it = std::find_if(myBackend->links.begin(), myBackend->links.end(), [&linkId](const auto& lhs)
		{
			return lhs.id == linkId;
		});

		if (it != myBackend->links.end())
		{
			myBackend->links.erase(it);
		}
	}

	void Editor::RemoveNode(const Volt::UUID nodeId)
	{
		auto it = std::find_if(myBackend->nodes.begin(), myBackend->nodes.end(), [&nodeId](const auto& lhs)
		{
			return lhs.id == nodeId;
		});

		if (it == myBackend->nodes.end())
		{
			return;
		}

		for (const auto& pin : it->pins)
		{
			auto lIt = std::find_if(myBackend->links.begin(), myBackend->links.end(), [&pin](const auto& lhs)
			{
				return lhs.input == pin || lhs.output == pin;
			});

			if (lIt != myBackend->links.end())
			{
				RemoveLink(lIt->id);
			}
		}

		myBackend->nodes.erase(it);
	}

	const std::vector<Volt::UUID> Editor::GetSelectedNodes()
	{
		std::vector<ed::NodeId> selectedNodeIds;
		selectedNodeIds.resize(ed::GetSelectedObjectCount());

		int32_t nodeCount = ed::GetSelectedNodes(selectedNodeIds.data(), (int32_t)selectedNodeIds.size());
		selectedNodeIds.resize(nodeCount);

		std::vector<Volt::UUID> result;
		for (const auto& n : myBackend->nodes)
		{
			auto it = std::find_if(selectedNodeIds.begin(), selectedNodeIds.end(), [&](const ed::NodeId& id)
			{
				return id.Get() == n.id;
			});

			if (it != selectedNodeIds.end())
			{
				result.emplace_back(n.id);
			}
		}

		return result;
	}

	const std::vector<Volt::UUID> Editor::GetSelectedLinks()
	{
		std::vector<ed::LinkId> selectedLinkIds;
		selectedLinkIds.resize(ed::GetSelectedObjectCount());

		int32_t linkCount = ed::GetSelectedLinks(selectedLinkIds.data(), (int32_t)selectedLinkIds.size());
		selectedLinkIds.resize(linkCount);

		std::vector<Volt::UUID> result;
		for (const auto& l : myBackend->links)
		{
			auto it = std::find_if(selectedLinkIds.begin(), selectedLinkIds.end(), [&](const ed::LinkId& id)
			{
				return id.Get() == l.id;
			});

			if (it != selectedLinkIds.end())
			{
				result.emplace_back(l.id);
			}
		}

		return result;
	}

	void Editor::InitializeEditor(Ref<EditorContext> context)
	{
		if (context)
		{
			myEditorContext = context;
		}

		if (myEditorContext->editorContext)
		{
			ed::DestroyEditor(myEditorContext->editorContext);
		}

		ax::NodeEditor::Config cfg{};
		cfg.SettingsFile = nullptr;
		cfg.UserPointer = this;

		cfg.SaveSettings = [](const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer)
		{
			Editor* editor = static_cast<Editor*>(userPointer);
			return editor->SaveSettings(data);
		};

		cfg.LoadSettings = [](char* data, void* userPointer) -> size_t
		{
			Editor* editor = static_cast<Editor*>(userPointer);

			std::string graphContext;
			editor->LoadSettings(graphContext);

			if (data)
			{
				memcpy_s(data, graphContext.size(), graphContext.c_str(), graphContext.size());
			}

			return graphContext.size();
		};

		cfg.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
		{
			Editor* editor = static_cast<Editor*>(userPointer);
			return editor->SaveNodeSettings(nodeId.Get(), data);
		};

		cfg.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
		{
			Editor* editor = static_cast<Editor*>(userPointer);

			std::string graphContext;
			editor->LoadNodeSettings(nodeId.Get(), graphContext);

			if (data)
			{
				memcpy_s(data, graphContext.size(), graphContext.c_str(), graphContext.size());
			}

			return graphContext.size();
		};

		myEditorContext->editorContext = ed::CreateEditor(&cfg);
		ed::SetCurrentEditor(myEditorContext->editorContext);
		ed::EnableShortcuts(true);

		InitializeStyle(ed::GetStyle());
	}

	void Editor::DrawEditor()
	{
		if (myHasDockSpace)
		{
			ImGui::SetNextWindowDockID(myMainDockId, ImGuiCond_Always);
			ImGui::SetNextWindowClass(GetWindowClass());

			ImGui::SetNextWindowSizeConstraints({ 100.f, 100.f }, { 0.f, 0.f });

			const std::string id = "Editor##" + myContext;
			ImGui::Begin(id.c_str());
		
			if (UI::DragDropTarget("ASSET_BROWSER_ITEM"))
			{

			}
		}

		ed::SetCurrentEditor(myEditorContext->editorContext);
		if (ed::Begin(myContext.c_str()))
		{
			DrawNodes();
			DrawLinks();

			if (!myCreateNewNode)
			{
				if (ed::BeginCreate({ 1.f, 1.f, 1.f, 1.f }, 2.f))
				{
					if (myEditorContext->onBeginCreate)
					{
						myEditorContext->onBeginCreate();
					}
				}
				else
				{
					myNewLinkPinId = 0;
				}

				ed::EndCreate();

				if (ed::BeginShortcut())
				{
					if (ed::AcceptCopy() && myEditorContext->onAcceptCopy)
					{
						myEditorContext->onAcceptCopy();
					}

					if (ed::AcceptCut() && myEditorContext->onAcceptCut)
					{
						myEditorContext->onAcceptCut();
					}

					if (ed::AcceptPaste() && myEditorContext->onAcceptPaste)
					{
						myEditorContext->onAcceptPaste();
					}

					if (ed::AcceptDuplicate() && myEditorContext->onAcceptDuplicate)
					{
						myEditorContext->onAcceptDuplicate();
					}
				}
				ed::EndShortcut();

				if (ed::BeginDelete())
				{
					ed::LinkId linkId;
					while (ed::QueryDeletedLink(&linkId))
					{
						if (ed::AcceptDeletedItem())
						{
							RemoveLink(linkId.Get());
							if (myEditorContext->onDeleteLink)
							{
								myEditorContext->onDeleteLink(linkId.Get());
							}
						}
					}

					ed::NodeId nodeId;
					while (ed::QueryDeletedNode(&nodeId))
					{
						if (ed::AcceptDeletedItem())
						{
							RemoveNode(nodeId.Get());
							if (myEditorContext->onDeleteNode)
							{
								myEditorContext->onDeleteNode(nodeId.Get());
							}
						}
					}
				}
				ed::EndDelete();
			}

			ed::Suspend();
			if (ed::ShowNodeContextMenu(&myEditorContext->contextNodeId) && myEditorContext->onShowNodeContextMenu)
			{
				myEditorContext->onShowNodeContextMenu();
			}

			if (ed::ShowPinContextMenu(&myEditorContext->contextPinId) && myEditorContext->onShowPinContextMenu)
			{
				myEditorContext->onShowPinContextMenu();
			}

			if (ed::ShowLinkContextMenu(&myEditorContext->contextLinkId) && myEditorContext->onShowLinkContextMenu)
			{
				myEditorContext->onShowLinkContextMenu();
			}

			if (ed::ShowBackgroundContextMenu() && myEditorContext->onShowBackgroundContextMenu)
			{
				myEditorContext->onShowBackgroundContextMenu();
			}

			DrawContextPopups();
			ed::Resume();
			ed::End();
			ed::SetCurrentEditor(nullptr);

			auto windowSize = ImGui::GetWindowSize();

			if (!myGraphTypeText.empty())
			{
				UI::ScopedFont font{ FontType::Bold_40 };
				UI::ScopedStyleFloat alpha{ ImGuiStyleVar_Alpha, 0.2f };

				const auto textSize = ImGui::CalcTextSize(myGraphTypeText.c_str());

				ImGui::SetCursorPos({ windowSize.x - textSize.x - 50.f, windowSize.y - textSize.y - 50.f });
				ImGui::TextUnformatted(myGraphTypeText.c_str());
			}
		}

		if (myHasDockSpace)
		{
			ImGui::End();
		}
	}

	void Editor::DrawLinks()
	{
		for (const auto& l : myBackend->links)
		{
			ed::Link(ed::LinkId(l.id), ed::PinId(l.output), ed::PinId(l.input));
		}
	}
}
