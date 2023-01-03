#pragma once
#include <Volt/Utility/Math.h>
#include <Volt/UI/Utility/UIMath.h>
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
		Element(const std::string aName) { name = aName;};
		 virtual ~Element(){};

		 const ElementType GetType() { return type; };

		gem::vec2 GetPosition() { return gem::vec2{ data.transform[3][0], data.transform[3][1] }; }
		void SetPosition(gem::vec2 aPosition)
		{
			//Covert position to normalized value
			gem::vec2 normalizedPosition = UIMath::GetNormalizedPosition(aPosition);

			data.transform[3][0] = normalizedPosition.x;
			data.transform[3][1] = normalizedPosition.y;
		}

		gem::mat4 GetWorldPosition()
		{
			if (parent)
			{
				gem::mat4 parentTransform = parent->GetWorldPosition();

				return data.transform * parentTransform;
			}

			return data.transform;
		}

		gem::mat4 GetTransform() { return data.transform;}
		gem::mat4 GetScaledTransform() { return data.transform * gem::scale(gem::mat4(1.0f), gem::vec3{ data.size.x * data.scale.x, data.size.y * data.scale.y, 1 }); }
		void SetTransform(gem::mat4 aTransform) { data.transform = aTransform; }

		gem::vec2 GetSize() { return data.size; }
		void SetSize(gem::vec2 aSize) { data.size = aSize; }

		gem::vec2 GetScale() { return data.scale; }
		void SetScale(gem::vec2 aScale) { data.scale = aScale; }

		gem::vec2 GetWorldScale()
		{
			if (parent)
			{
				gem::vec2 parentScale = parent->GetWorldScale();
				return data.scale * parentScale;
			}
			return data.scale;
		}

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
		void ReciveChild(Ref<UI::Element> aUIElement)
		{
			children.insert({ aUIElement->name, aUIElement });
			aUIElement->parent = this; 
		}

		Ref<UI::Element> GetChild(const std::string& aChildName)
		{
			auto child = children.find(aChildName);
			if (children.find(aChildName) != children.end())
			{
				return child->second;
			}
		}



		std::unordered_map<std::string, Ref<UI::Element>> children;

	protected:
		std::string name = "default";
		bool enabled = true;
		ElementType type = ElementType::UNASSIGNED;

		std::shared_ptr<UI::Canvas> canvas;

		elementData data;

		UI::Element* parent = nullptr;
	};
}

