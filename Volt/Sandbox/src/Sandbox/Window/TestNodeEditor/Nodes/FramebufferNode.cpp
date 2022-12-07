#include "sbpch.h"
#include "FramebufferNode.h"

#include <Volt/Utility/UIUtility.h>
#include <NodeEditor/Graph.h>

FramebufferNode::FramebufferNode()
	: NE::Node("Framebuffer")
{}

void FramebufferNode::OnCreate()
{
	Rebuild();
}

void FramebufferNode::DrawContent()
{
	if (UI::BeginProperties("framebuffer"))
	{
		UI::Property("Width", mySpecification.width);
		UI::Property("Height", mySpecification.height);

		UI::EndProperties();
	}

	ImGui::Separator();

	if (UI::TreeNodeWidth("Attachments", false, 2.f))
	{
		if (ImGui::Button("Add"))
		{
			mySpecification.attachments.emplace_back();
			mySpecification.attachments.back().debugName = "Attachment " + std::to_string(mySpecification.attachments.size());

			Rebuild();
		}

		for (uint32_t i = 0; auto & attachment : mySpecification.attachments)
		{
			std::string attId = attachment.debugName + "##att" + std::to_string(i);
			if (UI::TreeNodeWidth(attId, 200.f, 2.f))
			{
				const std::string propId = "attachmentProps" + std::to_string(i);
				if (UI::BeginProperties(propId))
				{
					UI::Property("Debug Name", attachment.debugName);
					UI::ComboProperty("Format", *(int32_t*)&attachment.format, myImageFormatStrings);
					UI::PropertyColor("Clear Color", attachment.clearColor);
					UI::Property("Readable", attachment.readable);
					UI::Property("Writable", attachment.writeable);
					UI::Property("Storage Compatible", attachment.storageCompatible);
					UI::Property("Is CubeMap", attachment.isCubeMap);

					UI::EndProperties();
				}
				UI::TreeNodePop();
			}

			i++;
		}

		UI::TreeNodePop();
	}
}

void FramebufferNode::Rebuild()
{
	myOutputs.clear();
	myInputs.clear();

	AddOutput("Output", NE::PinType::Framebuffer, &mySpecification);

	for (const auto& att : mySpecification.attachments)
	{
		AddInput(att.debugName, Volt::Utility::IsDepthFormat(att.format) ? NE::PinType::DepthAttachment : NE::PinType::ColorAttachment, &mySpecification);
	}
}
