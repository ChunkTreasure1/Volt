#pragma once
#include <Volt/Utility/Math.h>
#include <Volt/UI/Utility/Canvas.h>
#include <memory>
#include <unordered_map>

namespace UI 
{
	struct elementData
	{
		gem::mat4 transform = { 1.0f };
		gem::vec2 size = { 1.0f, 1.0f };
		gem::vec2 scale = { 1.0f, 1.0f };
		gem::vec2 pivot = { 0.0f, 0.0f };
		//eAnchored myAchor = eAnchored::C;
	};

	enum class ElementType
	{
		UNASSIGNED,
		SPRITE,
		COUNT
	};

	class Element
	{
	public:
		Element(const std::string aName) { name = aName; };
		~Element(){};

		gem::vec2 GetPosition() { return gem::vec2{ data.transform[3][0], data.transform[3][1] }; }
		void SetPosition(gem::vec2 aPosition)
		{
			//Covert position to normalized value
			//gem::vec2 normalizedPosition = UIMath::GetNormalizedPosition(aPosition);

			//Convert normalized position to Viewport position
			//gem::vec2 VP_position = UIMath::GetViewportPosition(normalizedPosition, *myCanvas);

			data.transform[3][0] = aPosition.x;
			data.transform[3][1] = aPosition.y;

			for (auto child : children)
			{
				child.second->UpdatePosition(*this);
			}
		}

		void UpdatePosition(UI::Element& aParentElement)
		{
			SetPosition(GetPosition() + aParentElement.GetPosition());

			for (auto child : children)
			{
				child.second->UpdatePosition(*this);
			}
		}

		gem::mat4 GetTransform() { return data.transform;}
		void SetTransform(gem::mat4 aTransform) { data.transform = aTransform; }

		gem::vec2 GetSize() { return data.size; }
		void SetSize(gem::vec2 aSize) { data.size = aSize; }

		gem::vec2 GetScale() { return data.scale; }
		void SetScale(gem::vec2 aScale) { data.scale = aScale; }

		gem::vec2 GetPivot() { return data.pivot; }
		void SetPivot(gem::vec2 aPivot) { data.pivot = aPivot; }

		void SetCanvas(std::shared_ptr<UI::Canvas> aCanvas) { canvas = aCanvas; SetChildCanvas(canvas); }
		std::shared_ptr<UI::Canvas> GetCanvas() { return canvas; }
		void SetChildCanvas(std::shared_ptr<UI::Canvas> aCanvas)
		{
			for (auto child : children)
			{
				child.second->SetCanvas(aCanvas);
				child.second->SetChildCanvas(aCanvas);
			}
		}
		void ReciveChild(UI::Element& aUIElement) { children.insert({ aUIElement.name, &aUIElement }); }

	private:
		std::string name = "default";
		bool enabled = true;
		ElementType type = ElementType::UNASSIGNED;

		std::shared_ptr<UI::Canvas> canvas;
		std::unordered_map<std::string, UI::Element*> children;

		elementData data;

	};
}

