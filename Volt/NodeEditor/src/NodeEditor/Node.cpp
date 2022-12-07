#include "nepch.h"
#include "Node.h"

#include "NodeEditor/Utility/DrawUtility.h"
#include "NodeEditor/Utility/ImGuiUtility.h"
#include "NodeEditor/Graph.h"

namespace NE
{
	Node::Node(const std::string& aName, ImColor aColor)
		: myName(aName), myColor(aColor)
	{}

	void Node::Draw()
	{
		ImVec2 headerMin, headerMax;

		ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
		ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
		ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
		ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

		ed::PushStyleVar(ed::StyleVar::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
		ed::PushStyleVar(ed::StyleVar_NodeRounding, 5.f);
		ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
		ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
		ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.f);
		ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
		ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);

		ed::BeginNode(myId);

		// Draw header
		{
			ImGui::Text(myName.c_str());

			headerMax = ImGui::GetItemRectMax();
			headerMin = ImGui::GetItemRectMin();
		}

		// Content
		{
			auto startPos = ImGui::GetCursorPos();
			float maxWidth = ImGui::GetCursorPos().x;
			if (maxWidth == startPos.x)
			{
				maxWidth += 150.f;
			}

			ImGui::SetCursorPos(startPos);

			for (const auto& input : myInputs)
			{
				ed::BeginPin(input->id, ax::NodeEditor::PinKind::Input);
				DrawPinIcon(*input, myGraph->PinHasConnection(input->id), 255);

				if (input->drawCallback)
				{
					input->drawCallback(*input, myGraph->PinHasConnection(input->id));
				}
				else
				{
					if (!input->name.empty())
					{
						ImGui::SameLine();
						ImGuiUtility::ShiftCursor(0.f, 3.f);
						ImGui::TextUnformatted(input->name.c_str());
					}
				}

				ed::EndPin();
			}

			if (!myOutputs.empty())
			{
				ImGui::SetCursorPos(startPos + ImVec2{ maxWidth - startPos.x, 0.f });
			}

			for (const auto& output : myOutputs)
			{
				ed::BeginPin(output->id, ax::NodeEditor::PinKind::Output);
				ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
				ed::PinPivotSize(ImVec2(0, 0));

				if (output->drawCallback)
				{
					output->drawCallback(*output, myGraph->PinHasConnection(output->id));
				}
				else
				{

					if (!output->name.empty())
					{
						ImGuiUtility::ShiftCursor(0.f, 3.f);
						ImGui::TextUnformatted(output->name.c_str());
						ImGui::SameLine();
						ImGuiUtility::ShiftCursor(0.f, -3.f);
					}
				}

				DrawPinIcon(*output, myGraph->PinHasConnection(output->id), 255);
				ed::EndPin();

				ImGui::SetCursorPos(startPos + ImVec2{ maxWidth - startPos.x, ImGui::GetCursorPos().y });
			}
		}

		ed::EndNode();

		// Draw header background
		{
			if (ImGui::IsItemVisible())
			{
				headerMax.x = ImGui::GetItemRectMax().x;

				auto drawList = ed::GetNodeBackgroundDrawList(myId);

				const auto halfBorderWidth = ed::GetStyle().NodeBorderWidth * 0.5f;

				if ((headerMax.x > headerMin.x) && (headerMax.y > headerMin.y))
				{
					drawList->AddRectFilled(headerMin - ImVec2(8 - halfBorderWidth, 4 - halfBorderWidth),
						headerMax - ImVec2(halfBorderWidth, 0.f), IM_COL32(255, 0, 255, 255), ed::GetStyle().NodeRounding);

					drawList->AddRectFilled(headerMin - ImVec2(8 - halfBorderWidth, -2.f),
						headerMax - ImVec2(halfBorderWidth, 0.f), IM_COL32(255, 0, 255, 255));

					const ImVec2 separatorMin = ImVec2(headerMin.x - 8 - halfBorderWidth, headerMax.y);
					const ImVec2 separatorMax = headerMax - ImVec2(halfBorderWidth, 0.f);

					drawList->AddLine(separatorMin, separatorMax, ImColor(32, 32, 32, 255));
				}
			}
		}

		ed::PopStyleVar(7);
		ed::PopStyleColor(4);
	}

	void Node::AddInput(const std::string& aName, PinType aType, void* aUserData, const std::function<void(const Pin&, bool)>& aDrawCallback /* = nullptr */)
	{
		auto input = myInputs.emplace_back(NE::Pin::Create(myGraph->GetId(), aName, aType, NE::PinMode::Input, aDrawCallback, aUserData));
		input->node = this;
	}

	void Node::AddOutput(const std::string& aName, PinType aType, void* aUserData, const std::function<void(const Pin&, bool)>& aDrawCallback /* = nullptr */)
	{
		auto output = myOutputs.emplace_back(NE::Pin::Create(myGraph->GetId(), aName, aType, NE::PinMode::Output, aDrawCallback, aUserData));
		output->node = this;
	}
}